/*
 * protocol_statemachine.h
 *
 *  Created on: Mar 22, 2025
 *      Author: fwar3
 */

#ifndef INC_PROTOCOL_STATEMACHINE_H_
#define INC_PROTOCOL_STATEMACHINE_H_

#include "protocol.h"
#include "receive_buffer.h"

typedef enum _DecodeState
{
	DECODE_RECV_HEADER,
	DECODE_RECV_BODY,
	DECODE_RECV_COMPLETE,
	DECODE_RECV_FILTER,
} DecodeState;

typedef void (*Callback)(ProtocolDecoder *protocolDecoder, ReceiveBuffer *receiveBuffer);

typedef struct _DecodeStateItem {
	DecodeState state;
	Callback callback;
} DecodeStateItem;

void StateMachineReset(ProtocolDecoder *protocolDecoder);
void ProcessData(ProtocolDecoder *protocolDecoder, ReceiveBuffer *receiveBuffer);

#endif /* INC_PROTOCOL_STATEMACHINE_H_ */
