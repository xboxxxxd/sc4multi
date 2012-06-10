#include "stdafx.h"

int _tmain(int argc, _TCHAR* argv[])
{
	// Create networking interfaces
	RakNet::RakPeerInterface *peer = RakNet::RakPeerInterface::GetInstance();
	RakNet::Packet *packet;

	// Initialize a socket descriptor
	RakNet::SocketDescriptor sd( 5187, 0 );

	// Start a server with max. 5 clients
	peer->Startup( 5, &sd, 1 );
	printf( "Starting the server...\n" );

	// Set the maximum number of clients
	peer->SetMaximumIncomingConnections( 5 );

	// Listen
	printf( "Listening...\n" );
	while( 1 )
	{
		for( packet = peer->Receive(); packet; peer->DeallocatePacket( packet ), packet = peer->Receive() )
		{
			switch( packet->data[0] )
			{
				case ID_NEW_INCOMING_CONNECTION:
					printf("A connection is incoming.\n");
					break;
				case ID_NO_FREE_INCOMING_CONNECTIONS:
					printf("The server is full.\n");
					break;
				case ID_DISCONNECTION_NOTIFICATION:
					printf("A client has disconnected.\n");
					break;
				case ID_CONNECTION_LOST:
					printf("A client lost the connection.\n");
					break;
				case ID_SC4_CONNECTION_DATA:
				{
					printf( "An SC4Multi connection was made:\n" );

					// Create several RakNet strings for reading the bistream
					RakNet::RakString username, guid, city;

					// Read the packet into the bitstream
					RakNet::BitStream bsIn(packet->data,packet->length,false);

					// Ignore the packet ID, which we already know
					bsIn.IgnoreBytes(sizeof(RakNet::MessageID));

					// Read the bitstream into our variables
					bsIn.Read( username );
					bsIn.Read( guid );
					bsIn.Read( city );

					// Print the data
					printf
					(
						"\tUsername | %s\n"
						"\tGUID     | %s\n"
						"\tCity     | %s\n",

						username.C_String(),
						guid.C_String(),
						city.C_String()
					);
					
					break;
				}
			
				default:
					printf("Message with identifier %i has arrived.\n", packet->data[0]);
					break;
			}
		}
	}

	// Shut down
	RakNet::RakPeerInterface::DestroyInstance( peer );

	return 0;
}