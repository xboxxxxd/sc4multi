#pragma once

// Version targeting
#include "targetver.h"

// Standard libraries
#include <stdio.h>
#include <tchar.h>

// Networking interfaces
#include <RakPeerInterface.h>
#include <MessageIdentifiers.h>
#include <BitStream.h>
#include <RakNetTypes.h>

// Game-specific (i.e. custom SC4Multi netcode) messages
enum GameMessages
{
	ID_SC4_CONNECTION_DATA = ID_USER_PACKET_ENUM + 1
};