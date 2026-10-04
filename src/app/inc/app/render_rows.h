#pragma once

#include <algorithm>
#include <atomic>
#include <exception>
#include <mutex>
#include <thread>
#include <vector>

namespace ray_tracer {

template <typename RenderRow>
auto ParallelRenderRows(const int height,
                        const RenderRow& render_row,
                        std::atomic<bool>* cancel = nullptr,
                        std::atomic<int>* progress = nullptr) -> bool {
  if (progress != nullptr) {
    progress->store(0);
  }
  if (cancel != nullptr && cancel->load()) {
    return false;
  }
  if (height <= 0) {
    return true;
  }
  const auto worker_count = std::min(static_cast<unsigned int>(height),
                                    std::max(1U, std::thread::hardware_concurrency()));
  std::atomic<int> next_row{0};
  std::atomic<int> completed_rows{0};
  std::atomic<bool> aborted{false};
  std::exception_ptr error;
  std::mutex error_mutex;
  const auto worker = [&]() {
    try {
      while (!aborted.load(std::memory_order_relaxed)) {
        if (cancel != nullptr && cancel->load()) {
          return;
        }
        const int row = next_row.fetch_add(1, std::memory_order_relaxed);
        if (row >= height) {
          return;
        }
        if (!render_row(row)) {
          aborted.store(true, std::memory_order_relaxed);
          return;
        }
        const int completed = completed_rows.fetch_add(1, std::memory_order_relaxed) + 1;
        if (progress != nullptr) {
          const int percent = (completed * 100) / height;
          int previous = progress->load(std::memory_order_relaxed);
          while (previous < percent &&
                 !progress->compare_exchange_weak(previous, percent, std::memory_order_relaxed)) {}
        }
      }
    } catch (...) {
      const std::lock_guard lock{error_mutex};
      if (!error) {
        error = std::current_exception();
      }
      aborted.store(true, std::memory_order_relaxed);
    }
  };
  {
    std::vector<std::jthread> workers;
    workers.reserve(worker_count - 1);
    for (unsigned int index = 1; index < worker_count; ++index) {
      workers.emplace_back(worker);
    }
    worker();
    // jthread joins every worker before the image can be returned or an error rethrown.
  }
  if (error) {
    std::rethrow_exception(error);
  }
  return completed_rows.load() == height && !aborted.load() && (cancel == nullptr || !cancel->load());
}

}  // namespace ray_tracer
