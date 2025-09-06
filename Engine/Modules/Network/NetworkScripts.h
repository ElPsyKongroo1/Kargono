#pragma once
#include "Modules/Assets/Asset.h"

namespace Kargono::Network
{
	struct ClientScripts
	{
		//=========================
		// Public Fields
		//=========================
		Assets::AssetHandle m_OnUpdateUserCount{ Assets::k_EmptyHandle };
		Assets::AssetHandle m_OnApproveJoinSession{ Assets::k_EmptyHandle };
		Assets::AssetHandle m_OnUserLeftSession{ Assets::k_EmptyHandle };
		Assets::AssetHandle m_OnCurrentSessionInit{ Assets::k_EmptyHandle };
		Assets::AssetHandle m_OnConnectionTerminated{ Assets::k_EmptyHandle };
		Assets::AssetHandle m_OnUpdateSessionUserSlot{ Assets::k_EmptyHandle };
		Assets::AssetHandle m_OnStartSession{ Assets::k_EmptyHandle };
		Assets::AssetHandle m_OnSessionReadyCheckConfirm{ Assets::k_EmptyHandle };
		Assets::AssetHandle m_OnReceiveSignal{ Assets::k_EmptyHandle };
	};
}