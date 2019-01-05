//
// 스트리밍에 관련된 전역을 선언한다.
//
#pragma once


namespace cvproc
{
	// 이미지 전송 프로토콜
	struct sStreamingData
	{
		BYTE id;				// streaming id (같은 아이디끼리 데이타를 합친 후에 출력한다.)
		BYTE chunkSize;	// chunk size (이미지 용량 때문에 나눠진 데이타 청크의 갯수)
		BYTE chunkIndex;	// chunk index
		BYTE isGray;		// gray 0 : 1
		BYTE isCompressed;	// jpeg compressed 0 : 1
		int imageBytes;			// image size (byte unit)
		BYTE *data;				// image data
	};


	// 리시버와 센더간에 정보 통신 프로토콜
	// TCP/IP 로만 전송된다.
	struct sStreamingProtocol
	{
		BYTE protocol;		// 프로토콜 타입
									// protocol = 100, 전송 프로토콜 설정 (TCP/IP) 
									// Receiver의 IP와 Port, 전송 타입 송신
									// 
									// protocol = 101, gray, compressed, compressed quality 설정
									// gray, compressed, compQuality 값 사용.
									//


		BYTE type;			// udp=1, tcp=0
		unsigned int uip;
		int port;
		bool gray;
		bool compressed;
		int compQuality;
		int fps;
	};


	const static int g_maxStreamSize = (int)pow(2, 15) - 1;
}
