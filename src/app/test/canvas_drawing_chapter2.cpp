#include <app/rendering/canvas.h>  // for Canvas
#include <app/materials/color.h>   // for color
#include <app/primitives/tuple.h>   // for tuple
#include <app/primitives/utilities.h>
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section
#include <fstream>
#include <string>

// Behavior Driven Development Style Testing
// https://github.com/catchorg/Catch2/blob/devel/docs/test-cases-and-sections.md#bdd-style-test-cases
namespace ray_tracer {
SCENARIO("colors are (red ,green, blue) tuples") {
  GIVEN("c is color(-0.5,0.4,1.7)") {
    THEN("c.red=-0.5 AND c.green = 0.4 AND c.blue = 1.7") {
      const ray_tracer::Color c_color{-0.5, 0.4, 1.7};
      CHECK(c_color.GetRed() == -0.5);
      CHECK(c_color.GetGreen() == 0.4);
      CHECK(c_color.GetBlue() == 1.7);
    }
  }
}

SCENARIO("Adding Colors") {
  GIVEN("c1 is color(0.9,0.6,0.75), c2 is (0.7,0.1,0.25)") {
    THEN("c1+c2 = color(1.6,0.7,1.0)") {
      const Color c1_color = Color(0.9, 0.6, 0.75);
      const Color c2_color = Color(0.7, 0.1, 0.25);
      CHECK(c1_color + c2_color == Color(1.6, 0.7, 1.0));
    }
  }
}
SCENARIO("Subtracting Colors") {
  GIVEN("c1 is color(0.9,0.6,0.75), c2 is (0.7,0.1,0.25)") {
    THEN("c1+c2 = color(0.2,0.5,0.5)") {
      const Color c1_color = Color(0.9, 0.6, 0.75);
      const Color c2_color = Color(0.7, 0.1, 0.25);
      CHECK(c1_color - c2_color == Color(0.2, 0.5, .5));
    }
  }
}
SCENARIO("Scalar Multiplication on Colors") {
  GIVEN("c1 is color(0.2,0.3,0.4)") {
    THEN("c1*2 = color(0.4,0.6,0.8)") {
      const Color c1_color = Color(0.2, 0.3, 0.4);
      CHECK(c1_color * 2 == Color(0.4, 0.6, .8));
    }
  }
}
SCENARIO("Hadamard Multiplication on Colors") {
  GIVEN("c1 is color(1,0.2,0.4),c2 = color(0.9,1,0.1") {
    THEN("c1*c2 = color(0.9,0.2,0.04)") {
      const Color c1_color = Color(1, 0.2, 0.4);
      const Color c2_color = Color(0.9, 1, 0.1);
      CHECK(c1_color * c2_color == Color(0.9, 0.2, 0.04));
    }
  }
}
SCENARIO("Creating a canvas ") {
  GIVEN("Given Canvas (10,20)") {
    THEN("c.width() = 10,c.height = 20") {
      Canvas const canvas(10, 20);
      REQUIRE(canvas.GetWidth() == 10);
      CHECK(canvas.AllPixelsColorCheck(Color(0, 0, 0)));
    }
  }
}
SCENARIO("Writing pixels to a canvas") {
  GIVEN("Given Canvas (10,20), and color(1,0,0) = red") {
    THEN("") {
      Canvas canvas(14, 20);
      const Color red{1, 0, 0};
      canvas.WritePixelColor(12, 3, red);
      CHECK(canvas.ColorAtPixel(12, 3) == red);
    }
  }
}
SCENARIO("Constructing PPM header") {
  GIVEN("c = canvas(5,3") {
    THEN("lines 1-3 of ppm are  P3  5 3  255 ") {
      Canvas canvas(5, 3);  // NOLINT
      const std::string ppm_file_header = "P3\n5 3\n255 ";
      const std::string filename{"/tmp/canvas.ppm"};
      canvas.CanvasToPPM("/tmp/canvas.ppm");
      CHECK(utilities::GetLines(1, 3, filename) == ppm_file_header);
    }
  }

  GIVEN("c = canvas(5,3) , c1 = color (1.5,0,0),c2 = color(0,0.5,0), c3 = color(-0.5,0,1)") {
    THEN("c.write(c,0,0,c1),c.write(2,1,c2),c.write(4,2,c3), ppm 4to 6 are set") {
      Canvas canvas(5, 3);
      Color const c1_color(1.5, 0, 0);
      Color const c2_color(0, 0.5, 0);
      Color const c3_color(-0.5, 0, 1);
      canvas.WritePixelColor(0, 0, c1_color);
      canvas.WritePixelColor(2, 1, c2_color);
      canvas.WritePixelColor(4, 2, c3_color);
      std::string const filename{"/tmp/canvas.ppm"};
      canvas.CanvasToPPM(filename);
      std::string fixed_string =
          "255 0 0 0 0 0 0 0 0 0 0 0 0 0 0 "
          "0 0 0 0 0 0 0 128 0 0 0 0 0 0 0 "
          "0 0 0 \n0 0 0 0 0 0 0 0 0 0 0 255 \n";
      CHECK(utilities::GetLines(4, 6, filename) == fixed_string);
    }
  }
}
SCENARIO("Splitting long lines in PPM files") {
  GIVEN("c =canvas(10,2), all pixels = color(1,0.8,0.6") {
    THEN("lines 4 -7 of ppm are fixed") {
      Canvas canvas{10, 2};
      for (auto x_dir = 0; x_dir < canvas.GetWidth(); x_dir++) {
        for (auto y_dir = 0; y_dir < canvas.GetHeight(); y_dir++) {
          canvas.WritePixelColor(x_dir, y_dir, Color(1, 0.8, 0.6));
        }
      }
      const std::string filename = "/tmp/canvas.ppm";
      canvas.CanvasToPPM(filename);
      std::string lines =
          "255 204 153 255 204 153 255 204 153 255 204 153 255 204 153 \n"
          "255 204 153 255 204 153 255 204 153 255 204 153 255 204 153 \n"
          "255 204 153 255 204 153 255 204 153 255 204 153 255 204 153 \n"
          "255 204 153 255 204 153 255 204 153 255 204 153 255 204 153 ";
      CHECK(utilities::GetLines(4, 7, filename) == lines);
    }
  }
}
SCENARIO("last line is new line character") {
  GIVEN("c =canvas(5,3)") {
    THEN("last line is new line character") {
      Canvas canvas{10, 2};
      const std::string filename = "/tmp/canvas.ppm";
      canvas.CanvasToPPM(filename);
      std::ifstream file_to_open(filename);
      file_to_open.seekg(-1, std::ios_base::end); // NOLINT
      char last_char;
      file_to_open.get(last_char);
      CHECK(last_char == '\n');
    }
  }
}
}  // namespace ray_tracer
