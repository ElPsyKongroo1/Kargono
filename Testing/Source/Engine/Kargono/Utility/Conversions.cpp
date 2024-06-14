#include "doctest.h"
#include "Kargono/Utility/Conversions.h"

TEST_CASE("Conversions::StringToBool")
{
	CHECK(Kargono::Utility::Conversions::StringToBool("True") == true);
	CHECK(Kargono::Utility::Conversions::StringToBool("False") == false);
	
	CHECK_THROWS_AS(Kargono::Utility::Conversions::StringToBool("asdfwe"), Kargono::TestingException);
	
}
