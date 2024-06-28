#include "doctest.h"
#include "Kargono/Utility/Conversions.h"

//TEST_CASE("Conversions::CharBufferToVariable")
//{
//	Kargono::Buffer testBuffer {};
//	SUBCASE("int32_t")
//	{
//		// Invalid State
//		CHECK(Kargono::Utility::Conversions::CharBufferToVariable(testBuffer) == true);
//
//	}
//	// Valid Set
//	// Invalid Set
//	CHECK_THROWS_AS(Kargono::Utility::Conversions::StringToBool(""), Kargono::TestingException);
//	CHECK_THROWS_AS(Kargono::Utility::Conversions::StringToBool("true"), Kargono::TestingException);
//	CHECK_THROWS_AS(Kargono::Utility::Conversions::StringToBool("false"), Kargono::TestingException);
//	CHECK_THROWS_AS(Kargono::Utility::Conversions::StringToBool("ahahahhaahhhahahhah"), Kargono::TestingException);
//
//}
TEST_CASE("Conversions::StringToBool")
{
	// Valid Set
	CHECK(Kargono::Utility::Conversions::StringToBool("True") == true);
	CHECK(Kargono::Utility::Conversions::StringToBool("False") == false);
	// Invalid Set
	CHECK_THROWS_AS(Kargono::Utility::Conversions::StringToBool(""), Kargono::TestingException);
	CHECK_THROWS_AS(Kargono::Utility::Conversions::StringToBool("true"), Kargono::TestingException);
	CHECK_THROWS_AS(Kargono::Utility::Conversions::StringToBool("false"), Kargono::TestingException);
	CHECK_THROWS_AS(Kargono::Utility::Conversions::StringToBool("ahahahhaahhhahahhah"), Kargono::TestingException);
}
