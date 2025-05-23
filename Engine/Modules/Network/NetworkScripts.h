#pragma once
#include "Modules/Assets/Asset.h"

namespace Kargono::Network
{
	struct ClientScripts
	{
		//=========================
		// Public Fields
		//=========================
		Assets::AssetHandle m_OnUpdateUserCount{ Assets::EmptyHandle };
		Assets::AssetHandle m_OnApproveJoinSession{ Assets::EmptyHandle };
		Assets::AssetHandle m_OnUserLeftSession{ Assets::EmptyHandle };
		Assets::AssetHandle m_OnCurrentSessionInit{ Assets::EmptyHandle };
		Assets::AssetHandle m_OnConnectionTerminated{ Assets::EmptyHandle };
		Assets::AssetHandle m_OnUpdateSessionUserSlot{ Assets::EmptyHandle };
		Assets::AssetHandle m_OnStartSession{ Assets::EmptyHandle };
		Assets::AssetHandle m_OnSessionReadyCheckConfirm{ Assets::EmptyHandle };
		Assets::AssetHandle m_OnReceiveSignal{ Assets::EmptyHandle };
	};
}