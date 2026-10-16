#pragma once
// Perlin "Improved Noise" — Ken Perlin, 2002
//
// Based on Ken Perlin's reference implementation from his SIGGRAPH 2002 paper:
//   "Improving Noise" — https://mrl.cs.nyu.edu/~perlin/paper445.pdf
//
// Reference Java implementation by Ken Perlin:
//   https://mrl.cs.nyu.edu/~perlin/noise/
//
// The permutation table, fade function (6t^5 - 15t^4 + 10t^3), and gradient
// selection are all from Perlin's original code. The 16-case gradient hash
// is described in detail in the paper (Section 3, "Implementation").
//
// Additional reference:
//   "The Book of Shaders: Noise" — https://thebookofshaders.com/11/
//   Adrian Biagioli's walkthrough — https://adrianb.io/2014/08/09/perlinnoise.html
//
// Key concepts:
//
//   Noise(x, y, z) — returns a smooth, continuous random value for any 3D
//   point. Nearby points produce similar values. A single call produces one
//   "octave" of noise: gentle, smooth variation with no fine detail.
//
//   Octave — one layer of noise at a specific frequency. The term is borrowed
//   from music where each octave doubles the frequency. In noise terms:
//     Octave 1: big smooth blobs      (frequency 1×, amplitude 1.0)
//     Octave 2: medium detail          (frequency 2×, amplitude 0.5)
//     Octave 3: smaller detail         (frequency 4×, amplitude 0.25)
//     ...each successive octave doubles frequency, halves amplitude.
//
//   Turbulence(x, y, z, octaves) — sums multiple octaves of noise together.
//   The low-frequency octaves define the overall shape, while high-frequency
//   octaves add fine-grained roughness at the edges. This is what produces
//   the rough, paint-like boundaries seen in perturbed patterns (as opposed
//   to the smooth wavy curves from a single octave).
//
//   Both concepts originate from Ken Perlin's 1985 SIGGRAPH paper
//   "An Image Synthesizer" which introduced procedural noise and the idea
//   of summing octaves for turbulence.

#include <array>
#include <cmath>

namespace ray_tracer {

class Perlin {
 public:
  [[nodiscard]] static auto Noise(double x_coord, double y_coord, double z_coord) -> double {
    const int x_int = static_cast<int>(std::floor(x_coord)) & 255;
    const int y_int = static_cast<int>(std::floor(y_coord)) & 255;
    const int z_int = static_cast<int>(std::floor(z_coord)) & 255;
    const double x_frac = x_coord - std::floor(x_coord);
    const double y_frac = y_coord - std::floor(y_coord);
    const double z_frac = z_coord - std::floor(z_coord);
    const double fade_x = Fade(x_frac);
    const double fade_y = Fade(y_frac);
    const double fade_z = Fade(z_frac);

    const auto& perm = kPerm;
    const int aaa = perm[perm[perm[x_int] + y_int] + z_int];
    const int aba = perm[perm[perm[x_int] + y_int + 1] + z_int];
    const int aab = perm[perm[perm[x_int] + y_int] + z_int + 1];
    const int abb = perm[perm[perm[x_int] + y_int + 1] + z_int + 1];
    const int baa = perm[perm[perm[x_int + 1] + y_int] + z_int];
    const int bba = perm[perm[perm[x_int + 1] + y_int + 1] + z_int];
    const int bab = perm[perm[perm[x_int + 1] + y_int] + z_int + 1];
    const int bbb = perm[perm[perm[x_int + 1] + y_int + 1] + z_int + 1];

    return Lerp(fade_z,
                Lerp(fade_y, Lerp(fade_x, Grad(aaa, x_frac, y_frac, z_frac), Grad(baa, x_frac - 1, y_frac, z_frac)),
                     Lerp(fade_x, Grad(aba, x_frac, y_frac - 1, z_frac), Grad(bba, x_frac - 1, y_frac - 1, z_frac))),
                Lerp(fade_y,
                     Lerp(fade_x, Grad(aab, x_frac, y_frac, z_frac - 1), Grad(bab, x_frac - 1, y_frac, z_frac - 1)),
                     Lerp(fade_x, Grad(abb, x_frac, y_frac - 1, z_frac - 1),
                          Grad(bbb, x_frac - 1, y_frac - 1, z_frac - 1))));
  }

  [[nodiscard]] static auto Turbulence(double x_coord, double y_coord, double z_coord, int octaves = 6) -> double {
    double value = 0;
    double freq = 1;
    double amp = 1;
    for (int oct = 0; oct < octaves; ++oct) {
      value += Noise(x_coord * freq, y_coord * freq, z_coord * freq) * amp;
      freq *= 2;
      amp *= 0.5;
    }
    return value;
  }

 private:
  static constexpr auto Fade(double val) -> double { return val * val * val * (val * (val * 6 - 15) + 10); }
  static constexpr auto Lerp(double val, double low, double high) -> double { return low + val * (high - low); }
  static constexpr auto Grad(int hash, double x_val, double y_val, double z_val) -> double {
    switch (hash & 0xF) {
      case 0x0: return x_val + y_val;
      case 0x1: return -x_val + y_val;
      case 0x2: return x_val - y_val;
      case 0x3: return -x_val - y_val;
      case 0x4: return x_val + z_val;
      case 0x5: return -x_val + z_val;
      case 0x6: return x_val - z_val;
      case 0x7: return -x_val - z_val;
      case 0x8: return y_val + z_val;
      case 0x9: return -y_val + z_val;
      case 0xA: return y_val - z_val;
      case 0xB: return -y_val - z_val;
      case 0xC: return y_val + x_val;
      case 0xD: return -y_val + z_val;
      case 0xE: return y_val - x_val;
      case 0xF: return -y_val - z_val;
      default: return 0;
    }
  }
  // Ken Perlin's original permutation table, doubled to avoid wrapping
  static constexpr std::array<int, 512> kPerm = {
      151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233, 7,   225, 140, 36,  103, 30,  69,  142,
      8,   99,  37,  240, 21,  10,  23,  190, 6,   148, 247, 120, 234, 75,  0,   26,  197, 62,  94,  252, 219, 203,
      117, 35,  11,  32,  57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125, 136, 171, 168, 68,  175, 74,  165,
      71,  134, 139, 48,  27,  166, 77,  146, 158, 231, 83,  111, 229, 122, 60,  211, 133, 230, 220, 105, 92,  41,
      55,  46,  245, 40,  244, 102, 143, 54,  65,  25,  63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,
      18,  169, 200, 196, 135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186, 3,   64,  52,  217, 226, 250,
      124, 123, 5,   202, 38,  147, 118, 126, 255, 82,  85,  212, 207, 206, 59,  227, 47,  16,  58,  17,  182, 189,
      28,  42,  223, 183, 170, 213, 119, 248, 152, 2,   44,  154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,
      129, 22,  39,  253, 19,  98,  108, 110, 79,  113, 224, 232, 178, 185, 112, 104, 218, 246, 97,  228, 251, 34,
      242, 193, 238, 210, 144, 12,  191, 179, 162, 241, 81,  51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,
      181, 199, 106, 157, 184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205, 93,  222, 114,
      67,  29,  24,  72,  243, 141, 128, 195, 78,  66,  215, 61,  156, 180,
      151, 160, 137, 91,  90,  15,  131, 13,  201, 95,  96,  53,  194, 233, 7,   225, 140, 36,  103, 30,  69,  142,
      8,   99,  37,  240, 21,  10,  23,  190, 6,   148, 247, 120, 234, 75,  0,   26,  197, 62,  94,  252, 219, 203,
      117, 35,  11,  32,  57,  177, 33,  88,  237, 149, 56,  87,  174, 20,  125, 136, 171, 168, 68,  175, 74,  165,
      71,  134, 139, 48,  27,  166, 77,  146, 158, 231, 83,  111, 229, 122, 60,  211, 133, 230, 220, 105, 92,  41,
      55,  46,  245, 40,  244, 102, 143, 54,  65,  25,  63,  161, 1,   216, 80,  73,  209, 76,  132, 187, 208, 89,
      18,  169, 200, 196, 135, 130, 116, 188, 159, 86,  164, 100, 109, 198, 173, 186, 3,   64,  52,  217, 226, 250,
      124, 123, 5,   202, 38,  147, 118, 126, 255, 82,  85,  212, 207, 206, 59,  227, 47,  16,  58,  17,  182, 189,
      28,  42,  223, 183, 170, 213, 119, 248, 152, 2,   44,  154, 163, 70,  221, 153, 101, 155, 167, 43,  172, 9,
      129, 22,  39,  253, 19,  98,  108, 110, 79,  113, 224, 232, 178, 185, 112, 104, 218, 246, 97,  228, 251, 34,
      242, 193, 238, 210, 144, 12,  191, 179, 162, 241, 81,  51,  145, 235, 249, 14,  239, 107, 49,  192, 214, 31,
      181, 199, 106, 157, 184, 84,  204, 176, 115, 121, 50,  45,  127, 4,   150, 254, 138, 236, 205, 93,  222, 114,
      67,  29,  24,  72,  243, 141, 128, 195, 78,  66,  215, 61,  156, 180};
};

}  // namespace ray_tracer
