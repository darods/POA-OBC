//
// 2016-02-07, jjuiddong
//
// 패킷을 저장한다. Thread Safe 
// 최대한 심플하게 만들었다.
//
// 큐가 가득차면, 가장 오래된 패킷을 제거하고, 추가한다.
// 각 소켓마다 packetSize 크기만큼 채울 때까지 다음으로 넘어가지 않는다.
//
// 패킷을 큐에 저장할 때, 헤더(sHeader)가 추가된다.
//
// 2016-02-07
//		- gcc 용 packetqueue 클래스.
//
#pragma once


#include "network.h"
#include "session.h"

namespace network
{

	struct sPacket
	{
		SOCKET sock; // 세션 소켓
		BYTE *buffer;
		int totalLen;
		bool full; // 버퍼가 다 채워지면 true가 된다.
		int readLen;
		int actualLen; // 실제 패킷의 크기를 나타낸다. totalLen - sizeof(sHeader)
	};

	struct sSession;
	class cPacketQueue
	{
	public:
		cPacketQueue();
		virtual ~cPacketQueue();

		struct sHeader
		{
			BYTE head[2]; // $@
			BYTE protocol; // protocol id
			int length;	// packet length (byte)
		};

		bool Init(const int packetSize, const int maxPacketCount);
		void Push(const SOCKET sock, const BYTE *data, const int len, const bool fromNetwork = false);
		bool Front(OUT sPacket &out);
		void Pop();
		void SendAll();
		void SendBroadcast(vector<sSession> &sessions, const bool exceptOwner = true);
		void Lock();
		void Unlock();

		vector<sPacket> m_queue;


	protected:
		//---------------------------------------------------------------------
		// Simple Queue Memory Pool
		BYTE* Alloc();
		void Free(BYTE*ptr);
		void ClearMemPool();
		void Clear();

		struct sChunk
		{
			bool used;
			BYTE *p;
		};
		vector<sChunk> m_memPool;
		BYTE *m_memPoolPtr;
		int m_packetBytes;	// 순수한 패킷 크기
		int m_chunkBytes;	// sHeader 헤더를 포함한 패킷 크기
		int m_totalChunkCount;
		pthread_mutex_t m_criticalSection;
	};
}
