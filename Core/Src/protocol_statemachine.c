#include "protocol_statemachine.h"
#include "protocol.h"
#include "usart.h"
#include <string.h>
#include <stdlib.h>

#define PROTOCOL_VERSION 0xAABB
#define HIGH_BYTE(word) ((word) >> 8)
#define LOW_BYTE(word) ((word) & 0xFF)

static void ProcessHeader(ProtocolDecoder *protocolDecoder, ReceiveBuffer *receiveBuffer);
static void ProcessBody(ProtocolDecoder *protocolDecoder, ReceiveBuffer *receiveBuffer);
static void ProcessFilter(ProtocolDecoder *protocolDecoder, ReceiveBuffer *receiveBuffer);

static DecodeStateItem stateTable[] = {
	{DECODE_RECV_FILTER, ProcessFilter},
	{DECODE_RECV_HEADER, ProcessHeader},
	{DECODE_RECV_BODY, ProcessBody},
};
static DecodeState currentState = DECODE_RECV_FILTER;  // 接收Header前先剔除非法数据

void StateMachineReset(ProtocolDecoder *protocolDecoder)
{
	UNUSED(protocolDecoder);
	currentState = DECODE_RECV_HEADER;
}

static void ProcessHeader(ProtocolDecoder *protocolDecoder, ReceiveBuffer *receiveBuffer)
{

	// 可以解码出完整MessageHeader
	if (protocolDecoder->decodeRecvLen + receiveBuffer->dataLen >= MESSAGE_HEADER_SIZE) {
		uint16_t readLen = MESSAGE_HEADER_SIZE - protocolDecoder->decodeRecvLen;
		memcpy(protocolDecoder->decodeBuffer + protocolDecoder->decodeWritePos, receiveBuffer->data, readLen); // 将MessageHeader拷贝完整
		protocolDecoder->decodeWritePos += readLen;
		protocolDecoder->decodeRecvLen += readLen;
		ConsumeData(receiveBuffer, readLen);
		protocolDecoder->currentMessageHeader = (MessageHeader*)(protocolDecoder->decodeBuffer + protocolDecoder->decodeWritePos - MESSAGE_HEADER_SIZE); // 指向MessageHeader地址
		currentState = DECODE_RECV_BODY; // 进入body处理
	} else { // 不能解码完整MessageHeader，继续等待数据包
		memcpy(protocolDecoder->decodeBuffer + protocolDecoder->decodeWritePos, receiveBuffer->consumePointer, receiveBuffer->dataLen);
		protocolDecoder->decodeWritePos += receiveBuffer->dataLen;
		protocolDecoder->decodeRecvLen += receiveBuffer->dataLen;
		ConsumeData(receiveBuffer, receiveBuffer->dataLen);
	}
}

static void ProcessBody(ProtocolDecoder *protocolDecoder, ReceiveBuffer *receiveBuffer)
{
	MessageHeader *messageHeader = protocolDecoder->currentMessageHeader;
	if (messageHeader == NULL) {
		return;
	}

	if (messageHeader->version != PROTOCOL_VERSION || messageHeader->bodyLength > DECODE_BUFFER_LENGTH) {
		// 清空ReceiveBuffer，重置状态机
		currentState = DECODE_RECV_FILTER;
		Clear(receiveBuffer);
		ProtocolReset(protocolDecoder);
		return;
	}

	// decode剩余的长度不够本次解码，则将decode buffer中的数据移动到前面后再处理
	if (protocolDecoder->decodeBufferLen - protocolDecoder->decodeWritePos < receiveBuffer->dataLen) {
		memmove(protocolDecoder->decodeBuffer,
				protocolDecoder->decodeBuffer + protocolDecoder->decodeWritePos - protocolDecoder->decodeRecvLen,
				protocolDecoder->decodeRecvLen);
		protocolDecoder->decodeWritePos = protocolDecoder->decodeRecvLen;
	}

	// body没有接收完整，继续等待数据包
	if (protocolDecoder->decodeRecvLen + receiveBuffer->dataLen < MESSAGE_HEADER_SIZE + messageHeader->bodyLength) {
		memcpy(protocolDecoder->decodeBuffer + protocolDecoder->decodeWritePos,
				receiveBuffer->consumePointer, receiveBuffer->dataLen);
		protocolDecoder->decodeWritePos += receiveBuffer->dataLen;
		protocolDecoder->decodeRecvLen += receiveBuffer->dataLen;
		ConsumeData(receiveBuffer, receiveBuffer->dataLen);
	} else {
		uint16_t readLen = MESSAGE_HEADER_SIZE + messageHeader->bodyLength - protocolDecoder->decodeRecvLen;
		uint8_t *msg = (uint8_t*)malloc(MESSAGE_HEADER_SIZE + messageHeader->bodyLength);
		memcpy(msg, messageHeader, protocolDecoder->decodeRecvLen);
		memcpy(msg + protocolDecoder->decodeRecvLen, receiveBuffer->consumePointer, readLen);
		WriteBufNotFull(&protocolDecoder->messageBuffer, (MessageHeader*)msg);

		ProtocolReset(protocolDecoder);
		ConsumeData(receiveBuffer, readLen);
		currentState = DECODE_RECV_FILTER;
	}
}

static void ProcessFilter(ProtocolDecoder *protocolDecoder, ReceiveBuffer *receiveBuffer)
{
	// 剔除decodeBuffer中的非法数据，合法的数据是version == 0xAABB，即decodeBuffer前两个字节是0xBB,0xAA
	uint8_t *data = protocolDecoder->decodeBuffer + protocolDecoder->decodeWritePos - protocolDecoder->decodeRecvLen;
	if (protocolDecoder->decodeRecvLen == 1 && data[0] != LOW_BYTE(PROTOCOL_VERSION)) {
		protocolDecoder->decodeRecvLen--;
		protocolDecoder->decodeWritePos = 0;
	}

	bool continueFilter = false;
	uint16_t indexMax = 0;
	if (protocolDecoder->decodeRecvLen > 0) {
		indexMax = protocolDecoder->decodeRecvLen - 1;
		for (uint8_t i = 0; i < indexMax; i++) {
			if (data[i] == LOW_BYTE(PROTOCOL_VERSION) && data[i + 1] == HIGH_BYTE(PROTOCOL_VERSION)) {
				currentState = DECODE_RECV_HEADER;
				break;
			} else {
				if (i == indexMax - 1) {
					continueFilter = true;
				}
				protocolDecoder->decodeRecvLen--;
			}
		}

		if (continueFilter && data[indexMax] != LOW_BYTE(PROTOCOL_VERSION)) {
			protocolDecoder->decodeRecvLen--;
			protocolDecoder->decodeWritePos = 0;
		}
	}

	if (protocolDecoder->decodeRecvLen == 0) {
		// 剔除receiveBuffer中的非法数据
		data = receiveBuffer->consumePointer;
		if (receiveBuffer->dataLen == 1 && data[0] != LOW_BYTE(PROTOCOL_VERSION)) {
			ConsumeData(receiveBuffer, 1);
		}

		if (receiveBuffer->dataLen > 0) {
			continueFilter = false;
			indexMax = receiveBuffer->dataLen - 1;
			for (uint16_t i = 0; i < indexMax; i++) {
				if (data[i] == LOW_BYTE(PROTOCOL_VERSION) && data[i + 1] == HIGH_BYTE(PROTOCOL_VERSION)) {
					currentState = DECODE_RECV_HEADER;
					break;
				} else {
					if (i == indexMax - 1) {
						continueFilter = true;
					}
					ConsumeData(receiveBuffer, 1);
				}
			}

			if (continueFilter && data[indexMax] != LOW_BYTE(PROTOCOL_VERSION)) {
				ConsumeData(receiveBuffer, 1);
			}
		}
	}
}

void ProcessData(ProtocolDecoder *protocolDecoder, ReceiveBuffer *receiveBuffer)
{
	while (!IsEmpty(receiveBuffer)) {
		for (uint8_t i = 0; i < sizeof(stateTable) / sizeof(stateTable[0]); ++i) {
			if (currentState == stateTable[i].state) {
				stateTable[i].callback(protocolDecoder, receiveBuffer);
			}
		}
	}
}
