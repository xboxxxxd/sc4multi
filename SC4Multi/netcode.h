// Shared includes
#include "Shared.h"

// Raknet specific
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include <RakNetTypes.h>

// Game-specific (i.e. custom SC4Multi netcode) messages
enum GameMessages
{
	ID_SC4_CONNECTION_DATA = ID_USER_PACKET_ENUM + 1
};

// The main function
DWORD WINAPI HandleNetworking( LPVOID );

// Stop running the networking instance
void StopRunningNet();