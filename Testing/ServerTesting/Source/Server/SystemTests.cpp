#include "doctest.h"

#include "Kargono/Utility/Timers.h"
#include "Kargono/Core/EngineCore.h"

TEST_CASE("Initialization and Termination")
{
	Kargono::Engine* core = nullptr;
	CHECK_NOTHROW(core = Kargono::InitEngineAndCreateApp({ 0, nullptr }));
	CHECK_NOTHROW(Kargono::Utility::AsyncBusyTimer::CloseAllTimers());
	CHECK_NOTHROW(delete core);
	core = nullptr;
}
