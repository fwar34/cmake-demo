#include "protocol.h"
#include "protocol_statemachine.h"
#include "usart.h"
#include <stdlib.h>
#include <stdio.h>

#define CHECK_NULL_PARAM(param)\
	if (param == NULL) {\
		return PROTOCOL_ERROR_INVALID_PARAM;\
	}

#define DECODE_BUFFER_LEN 2048

static ProtocolDecoder protocolDecoder;

ProtocolDecoder *GetProtocolDecoder()
{
	return &protocolDecoder;
}

int32_t ProtocolInit(ProtocolDecoder *protocolDecoder)
{
	CHECK_NULL_PARAM(protocolDecoder);

	InitBuf(&protocolDecoder->messageBuffer, MessageHeader*, MESSAGE_BUFFER_LENGTH);
	memset(protocolDecoder->decodeBuffer, 0, sizeof(protocolDecoder->decodeBuffer));
	protocolDecoder->decodeBufferLen = DECODE_BUFFER_LENGTH;
	protocolDecoder->decodeWritePos = 0;
	protocolDecoder->decodeRecvLen = 0;
	protocolDecoder->currentMessageHeader = NULL;
	Init(GetReceiveBuffer());

	return PROTOCOL_ERROR_OK;
}

int32_t ProtocolUninit(ProtocolDecoder *protocolDecoder)
{
	CHECK_NULL_PARAM(protocolDecoder);

	uint16_t remain = 0;
	do {
		MessageHeader *messageHeader = NULL;
		uint8_t ret = 0;
		ReadBuf(&protocolDecoder->messageBuffer, &messageHeader, &remain, &ret);
		if (ret != 0 && messageHeader != NULL) {
			free(messageHeader);
		}
	} while (remain > 0);

	protocolDecoder->decodeBufferLen = DECODE_BUFFER_LENGTH;
	protocolDecoder->decodeWritePos = 0;
	protocolDecoder->decodeRecvLen = 0;
	protocolDecoder->currentMessageHeader = NULL;

	return PROTOCOL_ERROR_OK;
}

void ProtocolReset(ProtocolDecoder *protocolDecoder)
{
	protocolDecoder->decodeBufferLen = DECODE_BUFFER_LENGTH;
	protocolDecoder->decodeWritePos = 0;
	protocolDecoder->decodeRecvLen = 0;
	protocolDecoder->currentMessageHeader = NULL;
}

void Decode(ProtocolDecoder *protocolDecoder, ReceiveBuffer *receiveBuffer)
{
	ProcessData(protocolDecoder, receiveBuffer);
}

MessageHeader *GetMessage(ProtocolDecoder *protocolDecoder, uint16_t *remainCount)
{
	UNUSED(protocolDecoder);
	UNUSED(remainCount);
	return NULL;
}

//uint16_t version;
//uint32_t bodyLength;
//uint32_t srcApp;
//uint32_t dstApp;
//uint16_t srcPort;
//uint16_t dstPort;
void DumpMessage(MessageHeader *messageHeader)
{
	// 使用 PRIx32 格式化 uint32_t 类型的参数
	UartPrintf("version: 0x%04x, bodyLength: 0x%" PRIx32 ", srcApp: 0x%" PRIx32 ", dstApp: 0x%" PRIx32 ", srcPort: 0x%04x, dstPort: 0x%04x\n",
			messageHeader->version, messageHeader->bodyLength, messageHeader->srcApp,
			messageHeader->dstApp, messageHeader->srcPort, messageHeader->dstPort);
}
