/*
 * protocol.h
 *
 *  Created on: Mar 22, 2025
 *      Author: fwar3
 */

#ifndef INC_PROTOCOL_H_
#define INC_PROTOCOL_H_

#include "error_define.h"
#include "circle_buffer.h"
#include "receive_buffer.h"
#include <inttypes.h>

#define MESSAGE_BUFFER_LENGTH 512
#define DECODE_BUFFER_LENGTH 1024

#define MESSAGE_HEADER_SIZE sizeof(MessageHeader)

#pragma pack(push)
#pragma pack(1)
typedef struct _MessageHeader
{
	uint16_t version;
	uint32_t bodyLength;
	uint32_t srcApp;
	uint32_t dstApp;
	uint16_t srcPort;
	uint16_t dstPort;
	// uint8_t data[0];
} MessageHeader;
#pragma pack(pop)

DefineCircleBuffer(MessageBuffer, MessageHeader*, MESSAGE_BUFFER_LENGTH);

typedef struct _ProtocolDecoder{
	MessageBuffer messageBuffer; // 解码完成的消息存放的循环数组
	uint8_t decodeBuffer[DECODE_BUFFER_LENGTH]; // 解码缓冲区
	uint32_t decodeBufferLen; // 解码缓冲区长度
	uint16_t decodeWritePos; // 解码缓冲区写索引
	uint16_t decodeRecvLen; // 解码缓冲区已接收的长度
	MessageHeader *currentMessageHeader; // 解码当前消息的头部索引
#if 0
	ReceiveBuffer receiveBuffer; // 接收缓冲区
#endif
} ProtocolDecoder;

ProtocolDecoder *GetProtocolDecoder();
int32_t ProtocolInit(ProtocolDecoder *protocolDecoder);
int32_t ProtocolUninit(ProtocolDecoder *protocolDecoder);
void ProtocolReset(ProtocolDecoder *protocolDecoder);
void Decode(ProtocolDecoder *protocolDecoder, ReceiveBuffer *receiveBuffer);
MessageHeader *GetMessage(ProtocolDecoder *protocolDecoder, uint16_t *remainCount);

void DumpMessage(MessageHeader *messageHeader);

#endif /* INC_PROTOCOL_H_ */
