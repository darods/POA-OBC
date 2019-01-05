
#include "udpserver.h"
#include "netlauncher.h"

using namespace std;
using namespace network;

void* UDPServerThreadFunction(void* arg);


cUDPServer::cUDPServer()
	: m_id(0)
	, m_socket(INVALID_SOCKET)
	, m_isConnect(false)
	, m_bufferLen(0)
	, m_maxBuffLen(BUFFER_LENGTH)
	, m_threadLoop(true)
	, m_sleepMillis(1)
{
	pthread_mutex_init(&m_CriticalSection, NULL);
	m_buffer = new BYTE[m_maxBuffLen];
}

cUDPServer::~cUDPServer()
{
	Close();

	pthread_mutex_destroy(&m_CriticalSection);
	SAFE_DELETEA(m_buffer);
}


bool cUDPServer::Init(const int id, const int port)
{
	m_id = id;
	m_port = port;

	if (m_isConnect)
	{
		close(m_socket);
		m_isConnect = false;
		m_threadLoop = false;
	}
	else
	{
		cout << "Bind UDP Server port = " << port << endl;

		if (network::LaunchUDPServer(port, m_socket))
		{
			m_isConnect = true;
			m_threadLoop = true;
			if (!m_handle)
			{
				pthread_create(&m_handle, NULL, UDPServerThreadFunction, this);
			}
		}
		else
		{
			return false;
		}
	}

	return true;
}


void cUDPServer::SetRecvData(const BYTE *buff, const int buffLen)
{
	pthread_mutex_lock(&m_CriticalSection);
	memcpy(m_buffer, buff, buffLen);
	m_bufferLen = buffLen;
	m_isReceiveData = true;
	pthread_mutex_unlock(&m_CriticalSection);
}


// 받은 패킷을 dst에 저장해서 리턴한다.
// 동기화 처리.
int cUDPServer::GetRecvData(OUT BYTE *dst, const int maxSize)
{
	pthread_mutex_lock(&m_CriticalSection);
	int buffLen = 0;
	if (maxSize < m_bufferLen)
	{
		pthread_mutex_unlock(&m_CriticalSection);
		return 0;
	}

	if (!m_isReceiveData || (m_bufferLen <= 0))
	{
		m_isReceiveData = false;
		pthread_mutex_unlock(&m_CriticalSection);
		return 0;
	}

	memcpy(dst, m_buffer, m_bufferLen);
	buffLen = m_bufferLen;
	m_isReceiveData = false;
	pthread_mutex_unlock(&m_CriticalSection);
	return buffLen;
}


void cUDPServer::Close(const bool isWait) // isWait = false
{
	m_isConnect = false;
	m_threadLoop = false;
	if (m_handle)
	{
		pthread_join(m_handle, NULL);
		m_handle = 0;
	}

	if (m_socket != INVALID_SOCKET)
	{
		close(m_socket);
		m_socket = INVALID_SOCKET;
	}
}


void cUDPServer::SetMaxBufferLength(const int length)
{
	if (m_maxBuffLen != length)
	{
		SAFE_DELETEA(m_buffer);

		m_maxBuffLen = length;
		m_buffer = new BYTE[length];
	}
}


// void PrintBuffer(const char *buffer, const int bufferLen)
// {
// 	for (int i = 0; i < bufferLen; ++i)
// 		printf("%c", buffer[i]);
// 	printf("\n");
// }


// UDP 서버 쓰레드
void* UDPServerThreadFunction(void* arg)
{
	cUDPServer *udp = (cUDPServer*)arg;

	BYTE *buff = new BYTE[udp->m_maxBuffLen];

	while (udp->m_threadLoop)
	{
		timeval t = { 0, udp->m_sleepMillis }; // ? millisecond
		fd_array readSockets;
		FD_ZERO(&readSockets);
		FD_SET(udp->m_socket, &readSockets);
		readSockets.fd_array[0] = udp->m_socket;
		const int maxfd = udp->m_socket + 1;

		const int ret = select(maxfd, &readSockets, NULL, NULL, &t);
		if (ret != 0 && ret != SOCKET_ERROR)
		{
			const int result = recv(readSockets.fd_array[0], buff, udp->m_maxBuffLen, 0);
			if (result == SOCKET_ERROR || result == 0) // 받은 패킷사이즈가 0이면 서버와 접속이 끊겼다는 의미다.
			{
				// 에러가 발생하더라도, 수신 대기상태로 계속 둔다.
			}
			else
			{
				udp->SetRecvData(buff, result);
			}

			//cout << "recv = " << result << ", maxbufflen=" << udp->m_maxBuffLen << endl;
		}
	}

	SAFE_DELETEA(buff);
	return 0;
}
