#include "doctest.h"

static int factorial(int number) { return number <= 1 ? number : factorial(number - 1) * number; }

TEST_CASE("Demo Test Case")
{
    CHECK(factorial(1) == 1);
    CHECK(factorial(2) == 2);
    CHECK(factorial(3) == 6);
	CHECK(factorial(10) == 3628800);
	CHECK(factorial(1) == 1);
}