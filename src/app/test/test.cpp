#include <app/app.h>         // for App
#include <catch2/catch_test_macros.hpp>  // for operator""_catch_sr, Section
#include <catch2/catch_tostring.hpp>     // for ReusableStringStream

// Behavior Driven Development Style Testing
// https://github.com/catchorg/Catch2/blob/devel/docs/test-cases-and-sections.md#bdd-style-test-cases
SCENARIO("addition function works as expected", "[addition]") {
  GIVEN("zeroes") {
    THEN("Zero behaves like identify") {
      CHECK(App::Add(0, 0) == 0);
      CHECK(App::Add(1, 0) == 1);
      CHECK(App::Add(0, 1) == 1);
    }
  }
  GIVEN("identical positive integers") {
    THEN("result is twice input") {
      for (int i = 0; i < 10; i++) {
        CHECK(App::Add(i, i) == 2 * i);
      }
    }
  }
  GIVEN("identical negative integers") {
    THEN("result is twice input") {
      for (int i = 0; i > -10; i--) {
        CHECK(App::Add(i, i) == 2 * i);
      }
    }
  }
  GIVEN("different integers") {
    WHEN("neg < pos") {
      THEN("result is negative") {
        for (int i = 0; i > 10; i--) {
          CHECK(App::Add(10, i) < -5);
          CHECK(App::Add(i, 10) < 0);
        }
      }
    }
    WHEN("neg > pos") {
      THEN("result is negative") {
        for (int i = 0; i < 10; i++) {
          CHECK(App::Add(-10, i) < 0);
          CHECK(App::Add(i, -10) < 0);
        }
      }
    }
  }
}
