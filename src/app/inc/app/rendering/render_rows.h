#pragma once

#include <algorithm>
#include <atomic>
#include <exception>
#include <mutex>
#include <thread>
#include <vector>

namespace ray_tracer {
namespace render_detail {

inline auto IsCancelled(const std::atomic<bool>* cancel) -> bool { return cancel != nullptr && cancel->load(); }

inline void UpdateRenderProgress(const int completed_rows, const int height, std::atomic<int>* progress) {
  if (progress == nullptr) {
    return;
  }

  const int percent = (completed_rows * 100) / height;
  int previous = progress->load(std::memory_order_relaxed);
  // Workers may report out of order; never let the displayed progress go backwards.
  while (previous < percent && !progress->compare_exchange_weak(previous, percent, std::memory_order_relaxed)) {
  }
}

}  // namespace render_detail

// render_row(row) returns true when the row finishes, or false to abort rendering.
template <typename RenderRow>
auto ParallelRenderRows(const int height,
                        const RenderRow& render_row,
                        std::atomic<bool>* cancel = nullptr,
                        std::atomic<int>* progress = nullptr) -> bool {
  if (progress != nullptr) {
    progress->store(0);
  }
  if (render_detail::IsCancelled(cancel)) {
    return false;
  }
  if (height <= 0) {
    return true;
  }
#ifdef __EMSCRIPTEN__
  // Leave two logical CPUs available for the browser and other work.
  const auto hardware_threads = std::thread::hardware_concurrency();
  const auto worker_limit = hardware_threads > 2U ? hardware_threads - 2U : 1U;
#else
  const auto worker_limit = std::max(1U, std::thread::hardware_concurrency());
#endif
  const auto worker_count = std::min(static_cast<unsigned int>(height), worker_limit);
  std::atomic<int> next_row{0};
  std::atomic<int> completed_rows{0};
  std::atomic<bool> aborted{false};
  std::exception_ptr first_exception;
  std::mutex exception_mutex;

  const auto render_available_rows = [&] -> auto {
    while (!aborted.load(std::memory_order_relaxed) && !render_detail::IsCancelled(cancel)) {
      // Claim a unique row. This counter does not synchronize pixel writes.
      const int row = next_row.fetch_add(1, std::memory_order_relaxed);
      if (row >= height) {
        return;
      }
      if (!render_row(row)) {
        aborted.store(true, std::memory_order_relaxed);
        return;
      }

      const int completed = completed_rows.fetch_add(1, std::memory_order_relaxed) + 1;
      render_detail::UpdateRenderProgress(completed, height, progress);
    }
  };

  // Exceptions must be caught inside each worker, then rethrown after all workers join.
  const auto run_worker = [&]() {
    try {
      render_available_rows();
    } catch (...) {
      const std::lock_guard lock{exception_mutex};
      if (!first_exception) {
        first_exception = std::current_exception();
      }
      aborted.store(true, std::memory_order_relaxed);
    }
  };

  {
    std::vector<std::jthread> workers;
    workers.reserve(worker_count - 1);
    for (unsigned int index = 1; index < worker_count; ++index) {
      workers.emplace_back(run_worker);
    }
    run_worker();  // The calling thread also renders rows.
    // Leaving this scope joins every jthread, making its pixel writes visible.
  }
  if (first_exception) {
    std::rethrow_exception(first_exception);
  }
  return completed_rows.load() == height && !aborted.load() && !render_detail::IsCancelled(cancel);
}

}  // namespace ray_tracer
