#include "doctest.h"

#include "Kargono/Utility/Timers.h"
#include "Kargono/Core/Engine.h"

TEST_CASE("Initialization and Termination")
{
	CHECK_NOTHROW(Kargono::InitEngineAndCreateApp({ 0, nullptr }));
	CHECK_NOTHROW(Kargono::Utility::AsyncBusyTimer::CloseAllTimers());
	CHECK_NOTHROW(Kargono::EngineService::Terminate());
}
