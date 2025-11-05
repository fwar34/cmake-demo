#include "receive_buffer.h"
#include <string.h>
#include <stdlib.h>

static ReceiveBuffer receiveBuffer;

ReceiveBuffer *GetReceiveBuffer()
{
	return &receiveBuffer;
}

void Init(ReceiveBuffer *receiveBuffer)
{
	memset(receiveBuffer, 0, sizeof(*receiveBuffer));
	receiveBuffer->capacity = RECEIVE_BUFFER_LENGTH;
}

bool IsEmpty(ReceiveBuffer *receiveBuffer)
{
	return receiveBuffer->dataLen == 0;
}

void ConsumeData(ReceiveBuffer* receiveBuffer, uint16_t consumeLen)
{
	receiveBuffer->consumePointer = receiveBuffer->data + consumeLen;
	receiveBuffer->dataLen -= consumeLen;
}

void Clear(ReceiveBuffer *receiveBuffer)
{
	memset(receiveBuffer, 0, sizeof(*receiveBuffer));
	receiveBuffer->capacity = RECEIVE_BUFFER_LENGTH;
}
