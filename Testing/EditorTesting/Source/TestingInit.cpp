#include "Kargono/Core/Base.h"
#include <filesystem>
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

int main(int argc, char** argv) {
	doctest::Context context;
	// Enable testing boolean
	Kargono::s_TestingActive = true;
	// Change to Editor Working Directory
	const std::filesystem::path relative_path = "../../Editor";
	std::filesystem::current_path(relative_path);
	Kargono::Log::Init();

	// defaults
	//context.addFilter("test-case-exclude", "*math*"); // exclude test cases with "math" in their name
	//context.setOption("abort-after", 5);              // stop test execution after 5 failed assertions
	context.setOption("order-by", "name");            // sort the test cases by their name

	context.applyCommandLine(argc, argv);

	// overrides
	context.setOption("no-breaks", true);             // don't break in the debugger when assertions fail

	int res = context.run(); // run

	if (context.shouldExit()) // important - query flags (and --exit) rely on the user doing this
		return res;          // propagate the result of the tests

	return res; // the result from doctest is propagated here as well
}

// Testing Principles

/*
 * Equivalence Partitioning: Discover sections of input that can be grouped together and likely react the same
 * as test cases.
 * Ex: If we are accepting a numeric value for a date, we know that values between 1-31 should be valid (for most months).
 *		This means that we can segment the input into three sections: -inf-0 (Invalid), 1-31 (Valid), and 32-inf (Invalid).
 *		These partitions can be tested once/ a few times since it is likely they will result in similar outcomes.
 *
 * Boundary Analysis: This works together with equivalence partitioning. The partitions discussed earlier have boundaries (for 1-31, 0,1,31,32 are boundaries)
 *		that are most likely to cause issues. Testing can focus on these boundaries in the partition to quickly resolve off-by-one errors.
 *
 * Decision Table: Table that lists all combinations of possible conditions for a specified system and provides the corresponding actions. These actions can be
 *		validated. Pretty much just a big truth table haha. These are useful for mapping out the decision space of a function if there are a lot of conditionals
 *		and specific results need to occur in particular cases.
 *
 */
