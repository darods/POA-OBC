
#include "udpclient.h"
#include "netlauncher.h"

using namespace std;
using namespace network;

void* UDPClientThreadFunction(void* arg);


cUDPClient::cUDPClient() 
	: m_isConnect(false)
, m_socket(INVALID_SOCKET)
, m_sndBuffLen(0)
, m_maxBuffLen(BUFFER_LENGTH)
, m_isSendData(false)
, m_handle(0)
, m_threadLoop(true)
, m_sleepMillis(30)
{
	pthread_mutex_init(&m_sndCriticalSection, NULL);

	m_sndBuffer = new BYTE[m_maxBuffLen];
}

cUDPClient::~cUDPClient()
{
	Close();
	
	pthread_mutex_destroy(&m_sndCriticalSection);

	SAFE_DELETEA(m_sndBuffer);
}


// UDP 클라언트 생성, ip, port 에 접속을 시도한다.
bool cUDPClient::Init(const string &ip, const int port, const int sleepMillis) //sleepMillis=30
{
	Close();

	m_ip = ip;
	m_port = port;
	m_sleepMillis = sleepMillis;

	if (network::LaunchUDPClient(ip, port, m_si_other, m_socket))
	{
		cout << "Connect UDP Client ip= " << ip << ", port= " << port << endl;

		m_isConnect = true;
		m_threadLoop = true;
		if (!m_handle)
		{
			pthread_create(&m_handle, NULL, UDPClientThreadFunction, this);
		}
	}
	else
	{
		cout << "Error!! Connect UDP Client ip=" << ip << ", port=" << port << endl;
		return false;
	}

	return true;
}


// 전송할 정보를 설정한다.
void cUDPClient::SendData(const BYTE *buff, const int buffLen)
{
	pthread_mutex_lock(&m_sndCriticalSection);
	m_sndBuffLen = min(buffLen, m_maxBuffLen);
	memcpy(m_sndBuffer, buff, m_sndBuffLen);
	m_isSendData = true;
	pthread_mutex_unlock(&m_sndCriticalSection);
}


void cUDPClient::SetMaxBufferLength(const int length)
{
	if (m_maxBuffLen != length)
	{
		SAFE_DELETEA(m_sndBuffer);

		m_maxBuffLen = length;
		m_sndBuffer = new BYTE[length];
	}
}


// 접속을 끊는다.
void cUDPClient::Close()
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


// UDP 네트워크 쓰레드.
void* UDPClientThreadFunction(void* arg)
{
	cUDPClient *udp = (cUDPClient*)arg;

	int recv_len;
	BYTE *sndBuff = new BYTE[udp->m_maxBuffLen];
	memset(sndBuff, '\0', udp->m_maxBuffLen);

	while (udp->m_threadLoop)
	{
		if (!udp->m_isConnect || (INVALID_SOCKET == udp->m_socket))
		{
			continue;
		}

		// Send
		bool isSend = false;
		pthread_mutex_lock(&udp->m_sndCriticalSection);
		if (udp->m_isSendData)
		{
			memcpy(sndBuff, udp->m_sndBuffer, udp->m_sndBuffLen);
			recv_len = udp->m_sndBuffLen;
			udp->m_isSendData = false;
			isSend = true;
		}
		pthread_mutex_unlock(&udp->m_sndCriticalSection);

		if (isSend)
		{
			const int slen = sizeof(udp->m_si_other);
			if (sendto(udp->m_socket, (char*)sndBuff,
				recv_len, 0, (struct sockaddr*) &udp->m_si_other, slen) == SOCKET_ERROR)
			{
				//printf("sendto() failed with error code : %d", WSAGetLastError());
				//exit(EXIT_FAILURE);
			}
		}

	}

	SAFE_DELETEA(sndBuff);
	return 0;
}
