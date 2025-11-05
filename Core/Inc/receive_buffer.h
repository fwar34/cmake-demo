/*
 * receive_buffer.h
 *
 *  Created on: Mar 22, 2025
 *      Author: fwar3
 */

#ifndef INC_RECEIVE_BUFFER_H_
#define INC_RECEIVE_BUFFER_H_

#include <inttypes.h>
#include <stdbool.h>

#define RECEIVE_BUFFER_LENGTH 1024

typedef struct _ReceiveBuffer
{
	uint8_t data[RECEIVE_BUFFER_LENGTH];
	uint8_t *consumePointer;
	uint16_t dataLen;
	uint16_t capacity;
} ReceiveBuffer;

ReceiveBuffer *GetReceiveBuffer();
void Init(ReceiveBuffer *receiveBuffer);
bool IsEmpty(ReceiveBuffer *receiveBuffer);
void ConsumeData(ReceiveBuffer* receiveBuffer, uint16_t consumeLen);
void Clear(ReceiveBuffer *receiveBuffer);

#endif /* INC_RECEIVE_BUFFER_H_ */
