#include "Shared.h"
#include "netcode.h"

// Are we connecting to a server?
bool isRunning = true;

void StopRunningNet()
{
	// Pretty straightforward, only affects HandleNetworking thread
	isRunning = false;
}

// Handles all networking events
DWORD WINAPI HandleNetworking( LPVOID )
{
	// Create RakNet interfaces
	RakNet::RakPeerInterface *peer;
	RakNet::Packet *packet;

	// Initialize networking
	peer = RakNet::RakPeerInterface::GetInstance();

	// Rev up your engines
	RakNet::SocketDescriptor sd( 0, "127.0.0.1" );
	peer->Startup( 1, &sd, 1 );

	// GO GO GO
	RakNet::ConnectionAttemptResult attempt = peer->Connect( "127.0.0.1", 5187, NULL, 0 );

	// Success?
	if( attempt != RakNet::CONNECTION_ATTEMPT_STARTED )
	{
		// Nope
		switch( attempt )
		{
			// None of these should happen, EVER.
			case RakNet::ALREADY_CONNECTED_TO_ENDPOINT:
				MessageBoxA
				(
					NULL,
					"A connection attempt was made to a server we're already connected to.\nPlease contact a developer.",
					"SC4Multi -- Network Error",
					MB_OK | MB_ICONERROR
				);
				break;
			case RakNet::CANNOT_RESOLVE_DOMAIN_NAME:
				MessageBoxA
				(
					NULL,
					"The given domain name could not be resolved.",
					"SC4Multi -- Network Error",
					MB_OK | MB_ICONERROR
				);
				break;
			case RakNet::CONNECTION_ATTEMPT_ALREADY_IN_PROGRESS:
				MessageBoxA
				(
					NULL,
					"Two connection attempts were being made at once by SC4Multi.\nPlease contact a developer.",
					"SC4Multi -- Network Error",
					MB_OK | MB_ICONERROR
				);
				break;
			case RakNet::INVALID_PARAMETER:
				MessageBoxA
				(
					NULL,
					"An invalid parameter was passed by SC4Multi. Please contact a developer.",
					"SC4Multi -- Network Error",
					MB_OK | MB_ICONERROR
				);
				break;
			case RakNet::SECURITY_INITIALIZATION_FAILED:
				MessageBoxA
				(
					NULL,
					"Security initialization failed. Something is terribly wrong,\nplease contact a developer.",
					"SC4Multi -- Network Error",
					MB_OK | MB_ICONERROR
				);
				break;
			default:
				MessageBoxA
				(
					NULL,
					"Something went wrong during network initialization,\nand we don't know what.",
					"SC4Multi -- Network Error",
					MB_OK | MB_ICONERROR
				);
				break;
		}

		// Terminate via ugly hacks
		exit( 1 );
	}

	// Forever and ever and ever and ever and ever and...
	while( isRunning )
	{
		// If there are any packets, read it
		for( packet = peer->Receive(); packet; peer->DeallocatePacket( packet ), packet = peer->Receive() )
		{
			// Get the packet type ID
			switch( packet->data[0] )
			{
				case ID_REMOTE_DISCONNECTION_NOTIFICATION:
					MessageBoxA( NULL, "Other client quit", "SC4Multi", MB_OK | MB_ICONINFORMATION );
					break;
				case ID_REMOTE_CONNECTION_LOST:
					MessageBoxA( NULL, "Other client timeout", "SC4Multi", MB_OK | MB_ICONINFORMATION );
					break;
				case ID_REMOTE_NEW_INCOMING_CONNECTION:
					MessageBoxA( NULL, "Other client connection", "SC4Multi", MB_OK | MB_ICONINFORMATION );
					break;
				case ID_CONNECTION_REQUEST_ACCEPTED:
				{
					MessageBoxA( NULL, "Our connection succeeded", "SC4Multi", MB_OK | MB_ICONINFORMATION );

					// Read the machine GUID generated by Windows
					char guid_net[255];
					bool isGuidNull = false;
					{
						// Create a wide-char GUID buffer
						wchar_t guid[255];

						// Registry stuff
						HKEY hKey    = 0;
						DWORD dwType = 0;

						// Get the size of our buffer
						DWORD dwBufSize = sizeof( guid );
						
						// Define the subkey
						const wchar_t* subkey = L"Software\\Microsoft\\Cryptography";
						
						// Open the registry
						if( RegOpenKey( HKEY_LOCAL_MACHINE, subkey, &hKey ) == ERROR_SUCCESS )
						{
							// We're looking for a string
							dwType = REG_SZ;

							// Read to our buffer
							if( !RegQueryValueEx( hKey, L"MachineGuid", 0, &dwType, (BYTE *)guid, &dwBufSize ) == ERROR_SUCCESS )
							{
								// Fall back to a null GUID
								isGuidNull = true;
							}
							else
							{
								// Convert the wide char buffer into a char buffer
								wcstombs( guid_net, guid, 255 );
							}
						}
						else
						{
							// Fall back to a null GUID
							isGuidNull = true;
						}
					}
					
					// Initialize a bitstream
					RakNet::BitStream bsOut;

					// Send our user data to the server
					{
						// Packet ID
						bsOut.Write( (RakNet::MessageID)ID_SC4_CONNECTION_DATA );

						// Nickname
						bsOut.Write( "Stormeus" );

						// Unique GUID, or a null one
						if( isGuidNull )
							bsOut.Write( 0 );
						else
						{
							#ifdef DEBUG
								MessageBoxA( NULL, guid_net, "SC4Multi -- GUID", MB_ICONINFORMATION | MB_OK );
							#endif

							bsOut.Write( guid_net );
						}

						// City taken
						bsOut.Write( "Unknown City" );
					}
					
					// Priority shipping to the server
					peer->Send
					(
						&bsOut,
						HIGH_PRIORITY,
						RELIABLE_ORDERED,
						0,
						packet->systemAddress,
						false
					);

					break;
				}
				case ID_NEW_INCOMING_CONNECTION:
					MessageBoxA( NULL, "Incoming connection", "SC4Multi", MB_OK | MB_ICONINFORMATION );
					break;
				case ID_NO_FREE_INCOMING_CONNECTIONS:
					MessageBoxA( NULL, "Server is full", "SC4Multi", MB_OK | MB_ICONINFORMATION );
					break;
				case ID_DISCONNECTION_NOTIFICATION:
					MessageBoxA( NULL, "We disconnected", "SC4Multi", MB_OK | MB_ICONINFORMATION );
					break;
				case ID_CONNECTION_LOST:
					MessageBoxA( NULL, "We lost connection", "SC4Multi", MB_OK | MB_ICONINFORMATION );
					break;
			}
		}
	}

	// Destroy this instance
	RakNet::RakPeerInterface::DestroyInstance( peer );

	// And we're done here.
	MessageBoxA( NULL, "Destroyed RakNet instance.", "SC4Multi -- Networking Debug", MB_OK | MB_ICONEXCLAMATION );

	return 1;
}