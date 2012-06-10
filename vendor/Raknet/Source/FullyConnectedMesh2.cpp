/// \file
///
/// This file is part of RakNet Copyright 2003 Jenkins Software LLC
///
/// Usage of RakNet is subject to the appropriate license agreement.

#include "NativeFeatureIncludes.h"
#if _RAKNET_SUPPORT_FullyConnectedMesh2==1

#include "FullyConnectedMesh2.h"
#include "RakPeerInterface.h"
#include "MessageIdentifiers.h"
#include "BitStream.h"
#include "RakAssert.h"
#include "GetTime.h"
#include "Rand.h"
#include "DS_OrderedList.h"

using namespace RakNet;

int FCM2ParticipantComp( const FullyConnectedMesh2::FCM2Participant &key, const FullyConnectedMesh2::FCM2Participant &data )
{
	if (key.fcm2Guid < data.fcm2Guid)
		return -1;
	if (key.fcm2Guid > data.fcm2Guid)
		return 1;
	return 0;
}

STATIC_FACTORY_DEFINITIONS(FullyConnectedMesh2,FullyConnectedMesh2);

FullyConnectedMesh2::FullyConnectedMesh2()
{
	startupTime=0;
	totalConnectionCount=0;
	ourFCMGuid=0;
	autoParticipateConnections=true;




	connectOnNewRemoteConnections=true;

	hostRakNetGuid=UNASSIGNED_RAKNET_GUID;
}
FullyConnectedMesh2::~FullyConnectedMesh2()
{
	Clear();
}
RakNetGUID FullyConnectedMesh2::GetConnectedHost(void) const
{
	if (ourFCMGuid==0)
		return UNASSIGNED_RAKNET_GUID;
	return hostRakNetGuid;
}
SystemAddress FullyConnectedMesh2::GetConnectedHostAddr(void) const
{
	if (ourFCMGuid==0)
		return UNASSIGNED_SYSTEM_ADDRESS;
	return rakPeerInterface->GetSystemAddressFromGuid(hostRakNetGuid);
}
RakNetGUID FullyConnectedMesh2::GetHostSystem(void) const
{
	if (ourFCMGuid==0)
		return rakPeerInterface->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS);

	return hostRakNetGuid;
}
bool FullyConnectedMesh2::IsHostSystem(void) const
{
	return GetHostSystem()==rakPeerInterface->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS);
}
void FullyConnectedMesh2::GetHostOrder(DataStructures::List<RakNetGUID> &hostList)
{
	hostList.Clear(true, _FILE_AND_LINE_);

	if (ourFCMGuid==0 || fcm2ParticipantList.Size()==0)
	{
		hostList.Push(rakPeerInterface->GetMyGUID(), _FILE_AND_LINE_);
		return;
	}

	FCM2Participant fcm2;
	fcm2.fcm2Guid=ourFCMGuid;
	fcm2.rakNetGuid=rakPeerInterface->GetMyGUID();

	DataStructures::OrderedList<FCM2Participant, FCM2Participant, FCM2ParticipantComp> olist;
	olist.Insert(fcm2, fcm2, true, _FILE_AND_LINE_);
	for (unsigned int i=0; i < fcm2ParticipantList.Size(); i++)
		olist.Insert(fcm2ParticipantList[i], fcm2ParticipantList[i], true, _FILE_AND_LINE_);

	for (unsigned int i=0; i < olist.Size(); i++)
	{
		hostList.Push(olist[i].rakNetGuid, _FILE_AND_LINE_);
	}
}
bool FullyConnectedMesh2::IsConnectedHost(void) const
{
	return GetConnectedHost()==rakPeerInterface->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS);
}
void FullyConnectedMesh2::SetAutoparticipateConnections(bool b)
{
	autoParticipateConnections=b;
}
void FullyConnectedMesh2::ResetHostCalculation(void)
{
	hostRakNetGuid=UNASSIGNED_RAKNET_GUID;
	startupTime=RakNet::GetTimeUS();
	totalConnectionCount=0;
	ourFCMGuid=0;
	for (unsigned int i=0; i < fcm2ParticipantList.Size(); i++)
		SendFCMGuidRequest(fcm2ParticipantList[i].rakNetGuid);
}
bool FullyConnectedMesh2::AddParticipantInternal( RakNetGUID rakNetGuid, FCM2Guid theirFCMGuid )
{
	for (unsigned int i=0; i < fcm2ParticipantList.Size(); i++)
	{
		if (fcm2ParticipantList[i].rakNetGuid==rakNetGuid)
		{
			if (theirFCMGuid!=0)
				fcm2ParticipantList[i].fcm2Guid=theirFCMGuid;
			return false;
		}
	}

	FCM2Participant participant;
	participant.rakNetGuid=rakNetGuid;
	participant.fcm2Guid=theirFCMGuid;
	fcm2ParticipantList.Push(participant,_FILE_AND_LINE_);

	SendFCMGuidRequest(rakNetGuid);

	return true;
}
void FullyConnectedMesh2::AddParticipant( RakNetGUID rakNetGuid )
{
	if (rakPeerInterface->GetConnectionState(rakPeerInterface->GetSystemAddressFromGuid(rakNetGuid))!=IS_CONNECTED)
	{
#ifdef DEBUG_FCM2
		printf("AddParticipant to %s failed (not connected)\n", rakNetGuid.ToString());
#endif
		return;
	}

	AddParticipantInternal(rakNetGuid,0);
}
void FullyConnectedMesh2::GetParticipantList(DataStructures::List<RakNetGUID> &participantList)
{
	participantList.Clear(true, _FILE_AND_LINE_);
	unsigned int i;
	for (i=0; i < fcm2ParticipantList.Size(); i++)
		participantList.Push(fcm2ParticipantList[i].rakNetGuid, _FILE_AND_LINE_);
}
bool FullyConnectedMesh2::HasParticipant(RakNetGUID participantGuid)
{
	unsigned int i;
	for (i=0; i < fcm2ParticipantList.Size(); i++)
	{
		if (fcm2ParticipantList[i].rakNetGuid==participantGuid)
			return true;
	}
	return false;
}
PluginReceiveResult FullyConnectedMesh2::OnReceive(Packet *packet)
{
	switch (packet->data[0])
	{
	case ID_REMOTE_NEW_INCOMING_CONNECTION:
		{
			if (connectOnNewRemoteConnections)
				ConnectToRemoteNewIncomingConnections(packet);
		}
		break;
	case ID_FCM2_REQUEST_FCMGUID:
		OnRequestFCMGuid(packet);
		return RR_STOP_PROCESSING_AND_DEALLOCATE;
	case ID_FCM2_RESPOND_CONNECTION_COUNT:
		OnRespondConnectionCount(packet);
		return RR_STOP_PROCESSING_AND_DEALLOCATE;
	case ID_FCM2_INFORM_FCMGUID:
		OnInformFCMGuid(packet);
		return RR_STOP_PROCESSING_AND_DEALLOCATE;
	case ID_FCM2_UPDATE_MIN_TOTAL_CONNECTION_COUNT:
		OnUpdateMinTotalConnectionCount(packet);
		return RR_STOP_PROCESSING_AND_DEALLOCATE;
	case ID_FCM2_NEW_HOST:
		if (packet->wasGeneratedLocally==false)
			return RR_STOP_PROCESSING_AND_DEALLOCATE;
		break;
	case ID_FCM2_VERIFIED_JOIN_START:
		return OnVerifiedJoinStart(packet);
	case ID_FCM2_VERIFIED_JOIN_CAPABLE:
		return OnVerifiedJoinCapable(packet);
	case ID_FCM2_VERIFIED_JOIN_FAILED:
		OnVerifiedJoinFailed(packet->guid);
		return RR_CONTINUE_PROCESSING;
	case ID_FCM2_VERIFIED_JOIN_ACCEPTED:
		OnVerifiedJoinAccepted(packet);
		return RR_CONTINUE_PROCESSING;
	case ID_FCM2_VERIFIED_JOIN_REJECTED:
		OnVerifiedJoinRejected(packet);
		return RR_CONTINUE_PROCESSING;

	case ID_NAT_TARGET_UNRESPONSIVE:
	case ID_NAT_TARGET_NOT_CONNECTED:
	case ID_NAT_CONNECTION_TO_TARGET_LOST:
		{
			RakNet::RakNetGUID g;
			RakNet::BitStream b(packet->data, packet->length, false);
			b.IgnoreBits(8); // Ignore the ID_...
			b.Read(g);
			UpdateVerifiedJoinInProgressMember(g, UNASSIGNED_RAKNET_GUID, JIPS_FAILED);
			return RR_CONTINUE_PROCESSING;
		}
	case ID_NAT_PUNCHTHROUGH_FAILED:
		UpdateVerifiedJoinInProgressMember(packet->guid, UNASSIGNED_RAKNET_GUID, JIPS_FAILED);
		return RR_CONTINUE_PROCESSING;
	}

	return RR_CONTINUE_PROCESSING;
}
void FullyConnectedMesh2::OnRakPeerStartup(void)
{
	Clear();
	startupTime=RakNet::GetTimeUS();
}
void FullyConnectedMesh2::OnAttach(void)
{
	Clear();
	// In case Startup() was called first
	if (rakPeerInterface->IsActive())
		startupTime=RakNet::GetTimeUS();
}
void FullyConnectedMesh2::OnRakPeerShutdown(void)
{
	Clear();
	startupTime=0;
}
void FullyConnectedMesh2::OnClosedConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, PI2_LostConnectionReason lostConnectionReason )
{
	(void) lostConnectionReason;
	(void) systemAddress;
	(void) rakNetGUID;

	unsigned int idx;
	idx=0;
	while (idx < joinsInProgress.Size())
	{
		if (joinsInProgress[idx]->requester==rakNetGUID)
		{
			Packet *p = AllocatePacketUnified(sizeof(MessageID)+sizeof(unsigned char));
			p->data[0]=ID_FCM2_VERIFIED_JOIN_FAILED;
			p->systemAddress=systemAddress;
			p->systemAddress.systemIndex=(SystemIndex)-1;
			p->guid=rakNetGUID;
			p->wasGeneratedLocally=true;
			rakPeerInterface->PushBackPacket(p, true);

			RakNet::OP_DELETE(joinsInProgress[idx], _FILE_AND_LINE_);
			joinsInProgress.RemoveAtIndex(idx);
		}
		else
		{
			idx++;
		}
	}

	UpdateVerifiedJoinInProgressMember(rakNetGUID, UNASSIGNED_RAKNET_GUID, JIPS_FAILED);

	for (idx=0; idx < fcm2ParticipantList.Size(); idx++)
	{
		if (fcm2ParticipantList[idx].rakNetGuid==rakNetGUID)
		{
			fcm2ParticipantList[idx]=fcm2ParticipantList[fcm2ParticipantList.Size()-1];
#ifdef DEBUG_FCM2
			printf("Popping participant %s\n", fcm2ParticipantList[fcm2ParticipantList.Size()-1].rakNetGuid.ToString());
#endif

			fcm2ParticipantList.Pop();
			if (rakNetGUID==hostRakNetGuid && ourFCMGuid!=0)
			{	
				if (fcm2ParticipantList.Size()==0)
				{
					hostRakNetGuid=rakPeerInterface->GetMyGUID();
					hostFCM2Guid=ourFCMGuid;
				}
				else
				{
					CalculateHost(&hostRakNetGuid, &hostFCM2Guid);
				}
				PushNewHost(hostRakNetGuid, rakNetGUID);
			}
			return;
		}
	}

}
RakNet::TimeUS FullyConnectedMesh2::GetElapsedRuntime(void)
{
	RakNet::TimeUS curTime=RakNet::GetTimeUS();
	if (curTime>startupTime)
		return curTime-startupTime;
	else
		return 0;
}
void FullyConnectedMesh2::OnNewConnection(const SystemAddress &systemAddress, RakNetGUID rakNetGUID, bool isIncoming)
{
	(void) isIncoming;
	(void) rakNetGUID;
	(void) systemAddress;

	UpdateVerifiedJoinInProgressMember(systemAddress, rakNetGUID, JIPS_CONNECTED);

	if (autoParticipateConnections)
		AddParticipant(rakNetGUID);
}
void FullyConnectedMesh2::OnFailedConnectionAttempt(Packet *packet, PI2_FailedConnectionAttemptReason failedConnectionAttemptReason)
{
	if (failedConnectionAttemptReason==FCAR_ALREADY_CONNECTED)
	{
		UpdateVerifiedJoinInProgressMember(packet->systemAddress, packet->guid, JIPS_CONNECTED);
	}
	else
	{
		UpdateVerifiedJoinInProgressMember(packet->systemAddress, UNASSIGNED_RAKNET_GUID, JIPS_FAILED);
	}
}
void FullyConnectedMesh2::Clear(void)
{
	fcm2ParticipantList.Clear(false, _FILE_AND_LINE_);

	for (unsigned int i=0; i < joinsInProgress.Size(); i++)
	{
		RakNet::OP_DELETE(joinsInProgress[i], _FILE_AND_LINE_);
	}
	joinsInProgress.Clear(true, _FILE_AND_LINE_);

	totalConnectionCount=0;
	ourFCMGuid=0;
	lastPushedHost=UNASSIGNED_RAKNET_GUID;
}
void FullyConnectedMesh2::PushNewHost(const RakNetGUID &guid, RakNetGUID oldHost)
{
	Packet *p = AllocatePacketUnified(sizeof(MessageID)+sizeof(oldHost));
	RakNet::BitStream bs(p->data,p->length,false);
	bs.SetWriteOffset(0);
	bs.Write((MessageID)ID_FCM2_NEW_HOST);
	bs.Write(oldHost);
	p->systemAddress=rakPeerInterface->GetSystemAddressFromGuid(guid);
	p->systemAddress.systemIndex=(SystemIndex)-1;
	p->guid=guid;
	p->wasGeneratedLocally=true;
	rakPeerInterface->PushBackPacket(p, true);

	lastPushedHost=guid;
}
void FullyConnectedMesh2::SendFCMGuidRequest(RakNetGUID rakNetGuid)
{
	if (rakNetGuid==rakPeerInterface->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS))
		return;

	RakNet::BitStream bsOut;
	bsOut.Write((MessageID)ID_FCM2_REQUEST_FCMGUID);
	if (ourFCMGuid==0)
	{
		bsOut.Write(false);
		bsOut.Write(GetElapsedRuntime());
	}
	else
	{
		bsOut.Write(true);
		bsOut.Write(totalConnectionCount);
		bsOut.Write(ourFCMGuid);
	}
	rakPeerInterface->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,rakNetGuid,false);
}
void FullyConnectedMesh2::SendOurFCMGuid(SystemAddress addr)
{
	RakNet::BitStream bsOut;
	bsOut.Write((MessageID)ID_FCM2_INFORM_FCMGUID);
	RakAssert(ourFCMGuid!=0); // Can't inform others of our FCM2Guid if it's unset!
	bsOut.Write(ourFCMGuid);
	bsOut.Write(totalConnectionCount);
	rakPeerInterface->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,addr,false);
}
void FullyConnectedMesh2::SendConnectionCountResponse(SystemAddress addr, unsigned int responseTotalConnectionCount)
{
	RakNet::BitStream bsOut;
	bsOut.Write((MessageID)ID_FCM2_RESPOND_CONNECTION_COUNT);
	bsOut.Write(responseTotalConnectionCount);
	rakPeerInterface->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,addr,false);
}
void FullyConnectedMesh2::AssignOurFCMGuid(void)
{
	// Only assigned once ever
	RakAssert(ourFCMGuid==0);
	unsigned int randomNumber = randomMT();
	randomNumber ^= (unsigned int) (RakNet::GetTimeUS() & 0xFFFFFFFF);
	randomNumber ^= (unsigned int) (rakPeerInterface->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS).g & 0xFFFFFFFF);
	ourFCMGuid |= randomNumber;
	uint64_t reponse64 = totalConnectionCount;
	ourFCMGuid |= reponse64<<32;
}
void FullyConnectedMesh2::CalculateHost(RakNetGUID *rakNetGuid, FCM2Guid *fcm2Guid)
{
	// Can't calculate host without knowing our own
	RakAssert(ourFCMGuid!=0);

	// Can't calculate host without being connected to anyone else
	RakAssert(fcm2ParticipantList.Size()>0);

	// Return the lowest value of all FCM2Guid
	FCM2Guid lowestFCMGuid=ourFCMGuid;
	//	SystemAddress associatedSystemAddress=UNASSIGNED_SYSTEM_ADDRESS;
	RakNetGUID associatedRakNetGuid=rakPeerInterface->GetGuidFromSystemAddress(UNASSIGNED_SYSTEM_ADDRESS);

	unsigned int idx;
	for (idx=0; idx < fcm2ParticipantList.Size(); idx++)
	{
		if (fcm2ParticipantList[idx].fcm2Guid!=0 && fcm2ParticipantList[idx].fcm2Guid<lowestFCMGuid)
		{
			lowestFCMGuid=fcm2ParticipantList[idx].fcm2Guid;
			associatedRakNetGuid=fcm2ParticipantList[idx].rakNetGuid;
		}
	}

	*rakNetGuid=associatedRakNetGuid;
	*fcm2Guid=lowestFCMGuid;
}
void FullyConnectedMesh2::OnRequestFCMGuid(Packet *packet)
{
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	bsIn.IgnoreBytes(sizeof(MessageID));
	bool hasRemoteFCMGuid=false;
	bsIn.Read(hasRemoteFCMGuid);
	RakNet::TimeUS senderElapsedRuntime=0;
	unsigned int remoteTotalConnectionCount=0;
	FCM2Guid theirFCMGuid=0;
	if (hasRemoteFCMGuid)
	{
		bsIn.Read(remoteTotalConnectionCount);
		bsIn.Read(theirFCMGuid);
	}
	else
	{
		bsIn.Read(senderElapsedRuntime);
	}
	AddParticipantInternal(packet->guid,theirFCMGuid);
	if (ourFCMGuid==0)
	{
		if (hasRemoteFCMGuid==false)
		{
			// Nobody has a fcmGuid

			RakNet::TimeUS ourElapsedRuntime = GetElapsedRuntime();
			if (ourElapsedRuntime>senderElapsedRuntime)
			{
				// We are probably host
				SendConnectionCountResponse(packet->systemAddress, 2);
			}
			else
			{
				// They are probably host
				SendConnectionCountResponse(packet->systemAddress, 1);
			}
		}
		else
		{
			// They have a fcmGuid, we do not
			IncrementTotalConnectionCount(remoteTotalConnectionCount+1);

			AssignOurFCMGuid();
			unsigned int idx;
			for (idx=0; idx < fcm2ParticipantList.Size(); idx++)
				SendOurFCMGuid(rakPeerInterface->GetSystemAddressFromGuid(fcm2ParticipantList[idx].rakNetGuid));
		}
	}
	else
	{
		if (hasRemoteFCMGuid==false)
		{
			// We have a fcmGuid they do not
			SendConnectionCountResponse(packet->systemAddress, totalConnectionCount+1);
		}
		else
		{
			// We both have fcmGuids
			IncrementTotalConnectionCount(remoteTotalConnectionCount);

			SendOurFCMGuid(packet->systemAddress);
		}
	}
	CalculateAndPushHost();
}
void FullyConnectedMesh2::OnRespondConnectionCount(Packet *packet)
{
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	bsIn.IgnoreBytes(sizeof(MessageID));
	unsigned int responseTotalConnectionCount;
	bsIn.Read(responseTotalConnectionCount);
	IncrementTotalConnectionCount(responseTotalConnectionCount);
	bool wasAssigned;
	if (ourFCMGuid==0)
	{
		wasAssigned=true;
		AssignOurFCMGuid();
	}
	else
		wasAssigned=false;

	// 1 is returned to give us lower priority, but the actual minimum is 2
	IncrementTotalConnectionCount(2);

	if (wasAssigned==true)
	{
		unsigned int idx;
		for (idx=0; idx < fcm2ParticipantList.Size(); idx++)
			SendOurFCMGuid(rakPeerInterface->GetSystemAddressFromGuid(fcm2ParticipantList[idx].rakNetGuid));
		CalculateAndPushHost();
	}
}
void FullyConnectedMesh2::OnInformFCMGuid(Packet *packet)
{
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	bsIn.IgnoreBytes(sizeof(MessageID));

	FCM2Guid theirFCMGuid;
	unsigned int theirTotalConnectionCount;
	bsIn.Read(theirFCMGuid);
	bsIn.Read(theirTotalConnectionCount);
	IncrementTotalConnectionCount(theirTotalConnectionCount);

	if (AddParticipantInternal(packet->guid,theirFCMGuid))
	{
		// 1/19/2010 - Relay increased total connection count in case new participant only connects to part of the mesh
		unsigned int idx;
		RakNet::BitStream bsOut;
		bsOut.Write((MessageID)ID_FCM2_UPDATE_MIN_TOTAL_CONNECTION_COUNT);
		bsOut.Write(totalConnectionCount);
		for (idx=0; idx < fcm2ParticipantList.Size(); idx++)
		{
			if (packet->guid!=fcm2ParticipantList[idx].rakNetGuid)
				rakPeerInterface->Send(&bsOut,HIGH_PRIORITY,RELIABLE_ORDERED,0,fcm2ParticipantList[idx].rakNetGuid,false);
		}
	}

	if (ourFCMGuid==0)
	{
		AssignOurFCMGuid();
		unsigned int idx;
		for (idx=0; idx < fcm2ParticipantList.Size(); idx++)
			SendOurFCMGuid(rakPeerInterface->GetSystemAddressFromGuid(fcm2ParticipantList[idx].rakNetGuid));
	}

	CalculateAndPushHost();
}
void FullyConnectedMesh2::OnUpdateMinTotalConnectionCount(Packet *packet)
{
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	bsIn.IgnoreBytes(sizeof(MessageID));
	unsigned int newMin;
	bsIn.Read(newMin);
	IncrementTotalConnectionCount(newMin);
}
void FullyConnectedMesh2::GetParticipantCount(unsigned int *participantListSize) const
{
	*participantListSize=fcm2ParticipantList.Size();
}

unsigned int FullyConnectedMesh2::GetParticipantCount(void) const
{
	return fcm2ParticipantList.Size();
}
void FullyConnectedMesh2::CalculateAndPushHost(void)
{
	RakNetGUID newHostGuid;
	FCM2Guid newFcmGuid;
	if (ParticipantListComplete())
	{
		CalculateHost(&newHostGuid, &newFcmGuid);
		if (newHostGuid!=lastPushedHost)
		{
			hostRakNetGuid=newHostGuid;
			hostFCM2Guid=newFcmGuid;
			PushNewHost(hostRakNetGuid, lastPushedHost);
		}
	}
}
bool FullyConnectedMesh2::ParticipantListComplete(void)
{
	for (unsigned int i=0; i < fcm2ParticipantList.Size(); i++)
	{
		if (fcm2ParticipantList[i].fcm2Guid==0)
			return false;
	}
	return true;
}
void FullyConnectedMesh2::IncrementTotalConnectionCount(unsigned int i)
{
	if (i>totalConnectionCount)
	{
		totalConnectionCount=i;
		//	printf("totalConnectionCount=%i\n",i);
	}
}
void FullyConnectedMesh2::SetConnectOnNewRemoteConnection(bool attemptConnection, RakNet::RakString pw)
{
	connectOnNewRemoteConnections=attemptConnection;
	connectionPassword=pw;
}

void FullyConnectedMesh2::ConnectToRemoteNewIncomingConnections(Packet *packet)
{
	unsigned int count;
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(MessageID));
	bsIn.Read(count);
	SystemAddress remoteAddress;
	RakNetGUID remoteGuid;
	char str[64];
	for (unsigned int i=0; i < count; i++)
	{
		bsIn.Read(remoteAddress);
		bsIn.Read(remoteGuid);
		remoteAddress.ToString(false,str);
		rakPeerInterface->Connect(str,remoteAddress.GetPort(),connectionPassword.C_String(),(int) connectionPassword.GetLength());
	}
}
unsigned int FullyConnectedMesh2::GetTotalConnectionCount(void) const
{
	return totalConnectionCount;
}
void FullyConnectedMesh2::StartVerifiedJoin(RakNetGUID client)
{
	// Assert is because there is no point calling StartVerifiedJoin() if this client is already a participant
	RakAssert(HasParticipant(client)==false);
	RakAssert(client!=rakPeerInterface->GetMyGUID());

	BitStream bsOut;
	bsOut.Write((MessageID) ID_FCM2_VERIFIED_JOIN_START);
	bsOut.WriteCasted<unsigned short>(fcm2ParticipantList.Size());
	unsigned int i;
	for (i=0; i < fcm2ParticipantList.Size(); i++)
	{
		bsOut.Write(fcm2ParticipantList[i].rakNetGuid);
		bsOut.Write(rakPeerInterface->GetSystemAddressFromGuid(fcm2ParticipantList[i].rakNetGuid));
	}
	SendUnified(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, client, false);
}
void FullyConnectedMesh2::RespondOnVerifiedJoinCapable(Packet *packet, bool accept, BitStream *additionalData)
{
	VerifiedJoinInProgress vjip;
	DecomposeJoinCapable(packet, &vjip);

	DataStructures::List<RakNetGUID> participatingMembersOnClientSucceeded;
	DataStructures::List<RakNetGUID> participatingMembersOnClientFailed;
	DataStructures::List<RakNetGUID> participatingMembersNotOnClient;
	DataStructures::List<RakNetGUID> clientMembersNotParticipatingSucceeded;
	DataStructures::List<RakNetGUID> clientMembersNotParticipatingFailed;
	CategorizeVJIP(&vjip,
		participatingMembersOnClientSucceeded,
		participatingMembersOnClientFailed,
		participatingMembersNotOnClient,
		clientMembersNotParticipatingSucceeded,
		clientMembersNotParticipatingFailed);


	RakAssert(participatingMembersOnClientFailed.Size()==0);
	RakAssert(participatingMembersNotOnClient.Size()==0);
	
	RakNet::BitStream bsOut;
	if (accept)
	{
		bsOut.Write((MessageID)ID_FCM2_VERIFIED_JOIN_ACCEPTED);
		bsOut.Write(packet->guid);
		
		// Tell client to disconnect from clientMembersNotParticipatingSucceeded 
		bsOut.WriteCasted<unsigned short>(clientMembersNotParticipatingSucceeded.Size());
		for (unsigned int i=0; i < clientMembersNotParticipatingSucceeded.Size(); i++)
			bsOut.Write(clientMembersNotParticipatingSucceeded[i]);
		
		// Tell client to call AddParticipant() for participatingMembersOnClientSucceeded
		bsOut.WriteCasted<unsigned short>(participatingMembersOnClientSucceeded.Size());
		for (unsigned int i=0; i < participatingMembersOnClientSucceeded.Size(); i++)
			bsOut.Write(participatingMembersOnClientSucceeded[i]);

		if (additionalData)
			bsOut.Write(additionalData);

		for (unsigned int i=0; i < fcm2ParticipantList.Size(); i++)
			SendUnified(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, fcm2ParticipantList[i].rakNetGuid, false);

		// Send to myself too, so the user is notified
		SendUnified(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, rakPeerInterface->GetMyGUID(), false);
	}
	else
	{
		// Tell client rejected, otherwise process the same as ID_FCM2_VERIFIED_JOIN_FAILED
		bsOut.Write((MessageID)ID_FCM2_VERIFIED_JOIN_REJECTED);
		bsOut.Write(additionalData);
	}

	SendUnified(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->guid, false);
}
void FullyConnectedMesh2::GetVerifiedJoinRequiredProcessingList(RakNetGUID host, DataStructures::List<SystemAddress> &addresses, DataStructures::List<RakNetGUID> &guids)
{
	addresses.Clear(true, _FILE_AND_LINE_);
	guids.Clear(true, _FILE_AND_LINE_);

	unsigned int curIndex = GetJoinsInProgressIndex(host);
	if (curIndex!=(unsigned int) -1)
	{
		VerifiedJoinInProgress *vjip = joinsInProgress[curIndex];
		unsigned int j;
		for (j=0; j < vjip->members.Size(); j++)
		{
			if (vjip->members[j].joinInProgressState==JIPS_PROCESSING)
			{
				addresses.Push(vjip->members[j].systemAddress, _FILE_AND_LINE_);
				guids.Push(vjip->members[j].guid, _FILE_AND_LINE_);
			}
		}
	}
}
void FullyConnectedMesh2::GetVerifiedJoinAcceptedAdditionalData(Packet *packet, bool *thisSystemAccepted, DataStructures::List<RakNetGUID> &systemsAccepted, BitStream *additionalData)
{
	systemsAccepted.Clear(true, _FILE_AND_LINE_);

	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(MessageID));
	RakNetGUID systemToAddGuid;
	bsIn.Read(systemToAddGuid);
	*thisSystemAccepted = systemToAddGuid == rakPeerInterface->GetMyGUID();
	unsigned short listSize;
	bsIn.Read(listSize);
	bsIn.IgnoreBytes(listSize*RakNetGUID::size());
	bsIn.Read(listSize);
	if (systemToAddGuid==rakPeerInterface->GetMyGUID())
	{
		for (unsigned short i=0; i < listSize; i++)
		{
			bsIn.Read(systemToAddGuid);
			systemsAccepted.Push(systemToAddGuid, _FILE_AND_LINE_);
		}
		systemsAccepted.Push(packet->guid, _FILE_AND_LINE_);
	}
	else
	{
		systemsAccepted.Push(systemToAddGuid, _FILE_AND_LINE_);
		bsIn.IgnoreBytes(listSize*RakNetGUID::size());
	}
	if (additionalData)
	{
		additionalData->Reset();
		additionalData->Write(bsIn);
	}
}
void FullyConnectedMesh2::GetVerifiedJoinRejectedAdditionalData(Packet *packet, BitStream *additionalData)
{
	RakNet::BitStream bsIn(packet->data, packet->length, false);
	bsIn.IgnoreBytes(sizeof(MessageID));
	if (additionalData)
	{
		additionalData->Reset();
		additionalData->Write(bsIn);
	}
}
PluginReceiveResult FullyConnectedMesh2::OnVerifiedJoinStart(Packet *packet)
{
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	bsIn.IgnoreBytes(sizeof(MessageID));

	unsigned short listSize;
	bsIn.Read(listSize);

	unsigned int curIndex = GetJoinsInProgressIndex(packet->guid);
	if (curIndex!=(unsigned int) -1)
	{
		// Got update to existing list

		VerifiedJoinInProgress *vjip = joinsInProgress[curIndex];
		if (vjip->sentResults==false)
		{
			// Got ID_FCM2_VERIFIED_JOIN_START twice before sending ID_FCM2_VERIFIED_JOIN_CAPABLE
			RakAssert(vjip->sentResults!=false);
			return RR_STOP_PROCESSING_AND_DEALLOCATE;
		}

		for (unsigned int i=0; i < vjip->members.Size(); i++)
		{
			vjip->members[i].workingFlag=false;
		}

		// Server has updated list of participants
		for (unsigned short i=0; i < listSize; i++)
		{
			VerifiedJoinInProgressMember vjipm;
			ReadVerifiedJoinInProgressMember(&bsIn, &vjipm);

			unsigned int j;
			if (vjipm.guid!=UNASSIGNED_RAKNET_GUID)
				j = GetVerifiedJoinInProgressMemberIndex(vjipm.guid, vjip);
			else
				j = GetVerifiedJoinInProgressMemberIndex(vjipm.systemAddress, vjip);
			if (j==(unsigned int)-1)
			{
				// New
				vjipm.workingFlag=true;
				vjipm.joinInProgressState=JIPS_PROCESSING;
				vjip->members.Push(vjipm, _FILE_AND_LINE_);
			}
			else
			{
				vjip->members[j].workingFlag=true;
			}
		}

		for (unsigned int i=0; i < vjip->members.Size(); i++)
		{
			if (vjip->members[i].workingFlag==false)
				vjip->members[i].joinInProgressState=JIPS_UNNECESSARY;
		}

		if (ProcessVerifiedJoinInProgressIfCompleted(vjip))
		{
			// Completed
			return RR_STOP_PROCESSING_AND_DEALLOCATE;
		}

		// Else tell user about new list
		return RR_CONTINUE_PROCESSING;
	}

	VerifiedJoinInProgress *vjip = RakNet::OP_NEW<VerifiedJoinInProgress>(_FILE_AND_LINE_);
	vjip->requester=packet->guid;
	if (listSize==0)
	{
		vjip->sentResults=true;

		// Send back result
		RakNet::BitStream bsOut;
		bsOut.Write((MessageID)ID_FCM2_VERIFIED_JOIN_CAPABLE);
		bsOut.WriteCasted<unsigned short>(0);
		SendUnified(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->guid, false);
		vjip->sentResults=true;
		joinsInProgress.Push(vjip, _FILE_AND_LINE_);
		return RR_STOP_PROCESSING_AND_DEALLOCATE;
	}

	vjip->sentResults=false;

	for (unsigned short i=0; i < listSize; i++)
	{
		VerifiedJoinInProgressMember vjipm;
		ReadVerifiedJoinInProgressMember(&bsIn, &vjipm);
		vjip->members.Push(vjipm, _FILE_AND_LINE_);
	}
	joinsInProgress.Push(vjip, _FILE_AND_LINE_);
	return RR_CONTINUE_PROCESSING;
}
void FullyConnectedMesh2::DecomposeJoinCapable(Packet *packet, VerifiedJoinInProgress *vjip)
{
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	bsIn.IgnoreBytes(sizeof(MessageID));

	unsigned short listSize;
	bsIn.Read(listSize);

	for (unsigned short i=0; i < listSize; i++)
	{
		VerifiedJoinInProgressMember member;
		bsIn.Read(member.guid);
		bsIn.Read(member.systemAddress);
		bsIn.ReadCasted<unsigned char>(member.joinInProgressState);
		member.workingFlag=false;
		vjip->members.Push(member, _FILE_AND_LINE_);
	}
}
PluginReceiveResult FullyConnectedMesh2::OnVerifiedJoinCapable(Packet *packet)
{
	VerifiedJoinInProgress vjip;
	DecomposeJoinCapable(packet, &vjip);

	// If this assert hits, AddParticipant() was called on this system, or another system, which it should not have been.
	RakAssert(HasParticipant(packet->guid)==false);

	DataStructures::List<RakNetGUID> participatingMembersOnClientSucceeded;
	DataStructures::List<RakNetGUID> participatingMembersOnClientFailed;
	DataStructures::List<RakNetGUID> participatingMembersNotOnClient;
	DataStructures::List<RakNetGUID> clientMembersNotParticipatingSucceeded;
	DataStructures::List<RakNetGUID> clientMembersNotParticipatingFailed;
	CategorizeVJIP(&vjip,
		participatingMembersOnClientSucceeded,
		participatingMembersOnClientFailed,
		participatingMembersNotOnClient,
		clientMembersNotParticipatingSucceeded,
		clientMembersNotParticipatingFailed);

	if (participatingMembersOnClientFailed.Size()>0)
	{
		// Send ID_FCM2_VERIFIED_JOIN_FAILED with GUIDs to disconnect
		BitStream bsOut;
		bsOut.Write((MessageID) ID_FCM2_VERIFIED_JOIN_FAILED);
		SendUnified(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->guid, false);
		return RR_STOP_PROCESSING_AND_DEALLOCATE;
	}

	if (participatingMembersNotOnClient.Size()>0)
	{
		BitStream bsOut;
		bsOut.Write((MessageID) ID_FCM2_VERIFIED_JOIN_START);
		bsOut.WriteCasted<unsigned short>(participatingMembersNotOnClient.Size());
		unsigned int i;
		for (i=0; i < participatingMembersNotOnClient.Size(); i++)
		{
			bsOut.Write(participatingMembersNotOnClient[i]);
			bsOut.Write(rakPeerInterface->GetSystemAddressFromGuid(participatingMembersNotOnClient[i]));
		}
		SendUnified(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, packet->guid, false);
		return RR_STOP_PROCESSING_AND_DEALLOCATE;
	}

	// Let server decide if to accept or reject via RespondOnVerifiedJoinCapable
	return RR_CONTINUE_PROCESSING;
}
void FullyConnectedMesh2::OnVerifiedJoinFailed(RakNetGUID hostGuid)
{
	unsigned int curIndex = GetJoinsInProgressIndex(hostGuid);
	if (curIndex==(unsigned int) -1)
		return;

	VerifiedJoinInProgress *vjip = joinsInProgress[curIndex];
	for (unsigned int j=0; j < vjip->members.Size(); j++)
	{
		if (vjip->members[j].joinInProgressState!=JIPS_FAILED)
		{
			rakPeerInterface->CloseConnection(vjip->members[j].guid, true);
		}
	}

	// Clear joinsInProgress for packet->guid
	RakNet::OP_DELETE(joinsInProgress[curIndex], _FILE_AND_LINE_);
	joinsInProgress.RemoveAtIndex(curIndex);
}
void FullyConnectedMesh2::OnVerifiedJoinAccepted(Packet *packet)
{
	RakNet::BitStream bsIn(packet->data,packet->length,false);
	bsIn.IgnoreBytes(sizeof(MessageID));

	RakNetGUID systemToAddGuid;
	bsIn.Read(systemToAddGuid);

	if (systemToAddGuid==rakPeerInterface->GetMyGUID())
	{
		// My own system
		unsigned int curIndex = GetJoinsInProgressIndex(packet->guid);
		if (curIndex==(unsigned int)-1)
			return;

		unsigned short listSize;
		bsIn.Read(listSize);
		for (unsigned short i=0; i < listSize; i++)
		{
			// List of clientMembersNotParticipatingSucceeded
			RakNetGUID guid;
			bsIn.Read(guid);
			rakPeerInterface->CloseConnection(guid, true);
		}

		bsIn.Read(listSize);
		for (unsigned short i=0; i < listSize; i++)
		{
			// List of participatingMembersOnClientSucceeded
			RakNetGUID guid;
			bsIn.Read(guid);
			AddParticipant(guid);
		}
		AddParticipant(packet->guid);

		// Clear joinsInProgress for packet->guid
		RakNet::OP_DELETE(joinsInProgress[curIndex], _FILE_AND_LINE_);
		joinsInProgress.RemoveAtIndex(curIndex);
	}
	else
	{
		// Another system
		ConnectionState cs = rakPeerInterface->GetConnectionState(systemToAddGuid);
		RakAssert(cs==IS_CONNECTED);
		if (cs==IS_CONNECTED)
			AddParticipant(systemToAddGuid);
	}
}
void FullyConnectedMesh2::OnVerifiedJoinRejected(Packet *packet)
{
	OnVerifiedJoinFailed(packet->guid);
}
unsigned int FullyConnectedMesh2::GetJoinsInProgressIndex(RakNetGUID requester) const
{
	for (unsigned int i=0; i < joinsInProgress.Size(); i++)
	{
		if (joinsInProgress[i]->requester==requester)
			return i;
	}
	return (unsigned int) -1;
}
void FullyConnectedMesh2::UpdateVerifiedJoinInProgressMember(const AddressOrGUID systemIdentifier, RakNetGUID guidToAssign, FullyConnectedMesh2::JoinInProgressState newState)
{
	bool anythingChanged;

	for (unsigned int i=0; i < joinsInProgress.Size(); i++)
	{
		VerifiedJoinInProgress *vjip = joinsInProgress[i];
		if (vjip->sentResults==true)
			continue;
		anythingChanged=false;

		unsigned int j;
		j = GetVerifiedJoinInProgressMemberIndex(systemIdentifier, vjip);
		if (j!=(unsigned int)-1)
		{
			if (vjip->members[j].guid==UNASSIGNED_RAKNET_GUID && guidToAssign!=UNASSIGNED_RAKNET_GUID)
				vjip->members[j].guid = guidToAssign;

			if (vjip->members[j].joinInProgressState!=newState)
			{
				anythingChanged=true;
				vjip->members[j].joinInProgressState=newState;
			}
		}

		if (anythingChanged)
		{
			ProcessVerifiedJoinInProgressIfCompleted(vjip);
		}
	}
}
bool FullyConnectedMesh2::ProcessVerifiedJoinInProgressIfCompleted(VerifiedJoinInProgress *vjip)
{
	if (vjip->sentResults)
		return true;

	// If no systems in processing state, send results to server
	// Return true if this was done
	bool anyProcessing=false;
	for (unsigned int i=0; i < vjip->members.Size(); i++)
	{
		if (vjip->members[i].joinInProgressState==JIPS_PROCESSING)
		{
			anyProcessing=true;
			break;
		}
	}

	if (anyProcessing==true)
		return false;

	// Send results to server
	BitStream bsOut;
	bsOut.Write((MessageID) ID_FCM2_VERIFIED_JOIN_CAPABLE);
	bsOut.WriteCasted<unsigned short>(vjip->members.Size());
	unsigned int i;
	for (i=0; i < vjip->members.Size(); i++)
	{
		bsOut.Write(vjip->members[i].guid);
		bsOut.Write(vjip->members[i].systemAddress);
		bsOut.WriteCasted<unsigned char>(vjip->members[i].joinInProgressState);
	}
	SendUnified(&bsOut, HIGH_PRIORITY, RELIABLE_ORDERED, 0, vjip->requester, false);

	vjip->sentResults=true;
	return true;
}

void FullyConnectedMesh2::ReadVerifiedJoinInProgressMember(RakNet::BitStream *bsIn, VerifiedJoinInProgressMember *vjipm)
{
	bsIn->Read(vjipm->guid);
	bsIn->Read(vjipm->systemAddress);
	ConnectionState cs = rakPeerInterface->GetConnectionState(vjipm->guid);
	if (cs==IS_CONNECTED)
		vjipm->joinInProgressState=JIPS_CONNECTED;
	else if (cs==IS_DISCONNECTING || cs==IS_SILENTLY_DISCONNECTING)
		vjipm->joinInProgressState=JIPS_FAILED;
	else
		vjipm->joinInProgressState=JIPS_PROCESSING;
}

unsigned int FullyConnectedMesh2::GetVerifiedJoinInProgressMemberIndex(const AddressOrGUID systemIdentifier, VerifiedJoinInProgress *vjip)
{
	for (unsigned int j=0; j < vjip->members.Size(); j++)
	{
		if ((systemIdentifier.rakNetGuid!=UNASSIGNED_RAKNET_GUID && vjip->members[j].guid==systemIdentifier.rakNetGuid) ||
			(systemIdentifier.systemAddress!=UNASSIGNED_SYSTEM_ADDRESS && vjip->members[j].systemAddress==systemIdentifier.systemAddress))
		{
			return j;
		}
	}
	return (unsigned int) -1;
}

void FullyConnectedMesh2::CategorizeVJIP(VerifiedJoinInProgress *vjip,
										 DataStructures::List<RakNetGUID> &participatingMembersOnClientSucceeded,
										 DataStructures::List<RakNetGUID> &participatingMembersOnClientFailed,
										 DataStructures::List<RakNetGUID> &participatingMembersNotOnClient,
										 DataStructures::List<RakNetGUID> &clientMembersNotParticipatingSucceeded,
										 DataStructures::List<RakNetGUID> &clientMembersNotParticipatingFailed)
{
	for (unsigned int i=0; i < vjip->members.Size(); i++)
		vjip->members[i].workingFlag=false;

	for (unsigned int i=0; i < fcm2ParticipantList.Size(); i++)
	{
		unsigned int j = GetVerifiedJoinInProgressMemberIndex(fcm2ParticipantList[i].rakNetGuid, vjip);
		if (j==(unsigned int)-1)
		{
			participatingMembersNotOnClient.Push(fcm2ParticipantList[i].rakNetGuid, _FILE_AND_LINE_);
		}
		else
		{
			if (vjip->members[j].joinInProgressState==JIPS_FAILED)
				participatingMembersOnClientFailed.Push(fcm2ParticipantList[i].rakNetGuid, _FILE_AND_LINE_);
			else
				participatingMembersOnClientSucceeded.Push(fcm2ParticipantList[i].rakNetGuid, _FILE_AND_LINE_);
			vjip->members[j].workingFlag=true;
		}
	}


	for (unsigned int j=0; j < vjip->members.Size(); j++)
	{
		if (vjip->members[j].workingFlag==false)
		{
			if (vjip->members[j].joinInProgressState==JIPS_FAILED)
				clientMembersNotParticipatingFailed.Push(vjip->members[j].guid, _FILE_AND_LINE_);
			else
				clientMembersNotParticipatingSucceeded.Push(vjip->members[j].guid, _FILE_AND_LINE_);
		}
	}
}

#endif // _RAKNET_SUPPORT_*
