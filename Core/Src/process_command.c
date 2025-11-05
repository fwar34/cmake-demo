/*
 * process_command.c
 *
 *  Created on: Mar 23, 2025
 *      Author: fwar3
 */
#include "process_command.h"
#include "protocol.h"
#include <stdlib.h>

void ProcessCommand()
{
	MessageHeader *messageHeader = NULL;
	uint16_t remainCount = 0;
	uint8_t ret = 0;
	do {
		ReadBuf(&GetProtocolDecoder()->messageBuffer, &messageHeader, &remainCount, &ret);
		if (ret != 0) {
			DumpMessage(messageHeader);
			free(messageHeader);
		}
	} while (remainCount != 0);
}
