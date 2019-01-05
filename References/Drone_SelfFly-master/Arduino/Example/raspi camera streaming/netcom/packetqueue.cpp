
#include "network.h"
#include "session.h"
#include "packetqueue.h"

using namespace network;


cPacketQueue::cPacketQueue()
	: m_memPoolPtr(NULL)
	, m_chunkBytes(0)
	, m_totalChunkCount(0)
{
	pthread_mutex_init(&m_criticalSection, NULL);
}

cPacketQueue::~cPacketQueue()
{
	Clear();

	pthread_mutex_destroy(&m_criticalSection);
}


bool cPacketQueue::Init(const int packetSize, const int maxPacketCount)
{
	Clear();

	// Init Memory pool
	m_chunkBytes = packetSize;
	m_totalChunkCount = maxPacketCount;
	m_memPoolPtr = new BYTE[(packetSize + sizeof(sHeader)) * maxPacketCount];
	m_memPool.reserve(maxPacketCount);
	for (int i = 0; i < maxPacketCount; ++i)
		m_memPool.push_back({ false, m_memPoolPtr + (i*(packetSize + sizeof(sHeader))) });
	//

	m_queue.reserve(maxPacketCount);

	return true;
}


void cPacketQueue::Push(const SOCKET sock, const BYTE *data, const int len,
	const bool fromNetwork) // fromNetwork = false
{
	cAutoCS cs(m_criticalSection);

	bool isCopyBuffer = false;

	// find specific packet by socket
	for (u_int i = 0; i < m_queue.size(); ++i)
	{
		if (sock == m_queue[i].sock)
		{
			// 해당 소켓이 채워지지 않은 버퍼라면,
			if (m_queue[i].readLen < m_queue[i].totalLen)
			{
				const int copyLen = MIN(m_queue[i].totalLen - m_queue[i].readLen, len);
				memcpy(m_queue[i].buffer + m_queue[i].readLen, data, copyLen);
				m_queue[i].readLen += copyLen;

				if (m_queue[i].readLen == m_queue[i].totalLen)
					m_queue[i].full = true;

				isCopyBuffer = true;
				break;
			}
		}
	}

	// 새로 추가될 소켓의 패킷이라면
	if (!isCopyBuffer)
	{
		sPacket packet;
		packet.sock = sock;
		packet.totalLen = 0;
		packet.readLen = len;

		int offset = 0;
		if (fromNetwork)
		{
			// 네트워크를 통해 온 패킷이라면, 이미 패킷헤더가 포함된 상태다.
			// 전체 패킷 크기를 저장해서, 분리된 패킷인지를 판단한다.
			sHeader *pheader = (sHeader*)data;
			if ((pheader->head[0] == '$') && (pheader->head[1] == '@'))
			{
				packet.totalLen = pheader->length;
				packet.actualLen = pheader->length - sizeof(sHeader);

				if (len == packet.totalLen)
					packet.full = true;
				else
					packet.full = false;
			}
			else
			{
				// error occur!!
				// 패킷의 시작부가 아닌데, 시작부로 들어왔음.
				// 헤더부가 깨졌거나, 기전 버퍼가 Pop 되었음.
				// 무시하고 종료한다.
				std::cout << "error occur" << std::endl;
				return;
			}
		}

		packet.buffer = Alloc();

		if (!packet.buffer)
		{
			cs.Leave();
			Pop();
			cs.Enter();
			packet.buffer = Alloc();
		}

		if (!packet.buffer)
			return; // Error!! 

		if (!fromNetwork)
		{
			// 송신할 패킷을 추가할 경우, 
			// 패킷 헤더를 추가한다.
			packet.totalLen = len + sizeof(sHeader);
			packet.actualLen = len;
			packet.full = true;

			sHeader header;
			header.head[0] = '$';
			header.head[1] = '@';
			header.length = len + sizeof(sHeader);

			memcpy(packet.buffer, &header, sizeof(sHeader));
			offset += sizeof(header);
		}

		// 패킷 내용 저장
		memcpy(packet.buffer + offset, data, len);

		m_queue.push_back(packet);
	}
}


bool cPacketQueue::Front(OUT sPacket &out)
{
 	cAutoCS cs(m_criticalSection);
 	RETV(m_queue.empty(), false);
 	RETV(!m_queue[0].full, false);

	out.sock = m_queue[0].sock;
	out.buffer = m_queue[0].buffer + sizeof(sHeader); // 헤더부를 제외한 패킷 데이타를 리턴한다.
	out.readLen = m_queue[0].readLen;
	out.totalLen = m_queue[0].totalLen;
	out.actualLen = m_queue[0].actualLen;

	return true;
}


void cPacketQueue::Pop()
{
 	cAutoCS cs(m_criticalSection);
 	RET(m_queue.empty());

	Free(m_queue.front().buffer);
	common::rotatepopvector(m_queue, 0);
}


// 즉시 모든 패킷을 전송한다.
void cPacketQueue::SendAll()
{
	RET(m_queue.empty());

	cAutoCS cs(m_criticalSection);
	for (u_int i = 0; i < m_queue.size(); ++i)
	{
		send(m_queue[i].sock, (const char*)m_queue[i].buffer, m_queue[i].totalLen, 0);
		Free(m_queue[i].buffer);
	}
	m_queue.clear();
}


// exceptOwner 가 true 일때, 패킷을 보낸 클라이언트를 제외한 나머지 클라이언트들에게 모두
// 패킷을 보낸다.
void cPacketQueue::SendBroadcast(vector<sSession> &sessions, const bool exceptOwner)
{
	cAutoCS cs(m_criticalSection);

	for (u_int i = 0; i < m_queue.size(); ++i)
	{
		if (!m_queue[i].full)
			continue; // 다 채워지지 않은 패킷은 무시

		for (u_int k = 0; k < sessions.size(); ++k)
		{
			// exceptOwner가 true일 때, 패킷을 준 클라이언트에게는 보내지 않는다.
			const bool isSend = !exceptOwner || (exceptOwner && (m_queue[i].sock != sessions[k].socket));
			if (isSend)
				send(sessions[k].socket, (const char*)m_queue[i].buffer, m_queue[i].totalLen, 0);
		}
	}

	// 모두 전송한 후 큐를 비운다.
	for (u_int i = 0; i < m_queue.size(); ++i)
		Free(m_queue[i].buffer);
	m_queue.clear();

	ClearMemPool();
}


void cPacketQueue::Lock()
{
	pthread_mutex_lock(&m_criticalSection);
}


void cPacketQueue::Unlock()
{
	pthread_mutex_unlock(&m_criticalSection);
}


BYTE* cPacketQueue::Alloc()
{
	for (u_int i = 0; i < m_memPool.size(); ++i)
	{
		if (!m_memPool[i].used)
		{
			m_memPool[i].used = true;
			return m_memPool[i].p;
		}
	}
	return NULL;
}


void cPacketQueue::Free(BYTE*ptr)
{
	for (u_int i = 0; i < m_memPool.size(); ++i)
	{
		if (ptr == m_memPool[i].p)
		{
			m_memPool[i].used = false;
			break;
		}
	}
}


void cPacketQueue::Clear()
{
	SAFE_DELETEA(m_memPoolPtr);
	m_memPool.clear();
	m_queue.clear();
}


// 메모리 풀을 초기화해서, 어쩌다 생길지 모를 버그를 제거한다.
void cPacketQueue::ClearMemPool()
{
	for (u_int i = 0; i < m_memPool.size(); ++i)
		m_memPool[i].used = false;
}
