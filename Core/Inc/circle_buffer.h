#ifndef __CIRCLE_BUFFER_H_
#define __CIRCLE_BUFFER_H_

#include "spinlock.h"
#include <inttypes.h>
#include <string.h>

#define DefineCircleBuffer(structType, itemType, bufSize)\
typedef struct _##structType {\
	itemType dataBuf[bufSize];\
	uint16_t readIndex;\
	uint16_t writeIndex;\
	uint16_t count;\
	uint16_t capacity;\
	volatile SpinLock bufLock;\
} structType

#define InitBuf(buffer, itemType, bufSize)\
	(buffer)->readIndex = 0;\
	(buffer)->writeIndex = 0;\
	(buffer)->count = 0;\
	(buffer)->capacity = bufSize;\
	(buffer)->bufLock = 0;\
	memset((buffer)->dataBuf, 0, sizeof(itemType) * (buffer)->capacity);

#define ReadBuf(buffer, out, remain, ret) \
{\
	accquire_spinlock(&(buffer)->bufLock, 0);\
	if ((buffer)->count > 0) {\
		*(out) = (buffer)->dataBuf[(buffer)->readIndex];\
		(buffer)->count--;\
		*(remain) = (buffer)->count;\
		(buffer)->readIndex = ((buffer)->readIndex + 1) % (buffer)->capacity;\
		*(ret) = 1;\
	} else {\
		*(ret) = 0;\
	}\
	release_spinlock(&(buffer)->bufLock);\
}

// buffer满了继续添加，同时偏移readIndex
#define WriteBuf(buffer, data)\
{\
	accquire_spinlock(&(buffer)->bufLock, 0);\
	if ((buffer)->count >= (buffer)->capacity && (buffer)->writeIndex == (buffer)->readIndex) {\
		(buffer)->readIndex = ((buffer)->readIndex + 1) % (buffer)->capacity;\
	}\
	(buffer)->dataBuf[(buffer)->writeIndex] = (data);\
	(buffer)->writeIndex = ((buffer)->writeIndex + 1) % (buffer)->capacity;\
	(buffer)->count++;\
	if ((buffer)->count > (buffer)->capacity) {\
		(buffer)->count = (buffer)->capacity;\
	}\
	release_spinlock(&(buffer)->bufLock);\
}

// buffer满了就不操作
#define WriteBufNotFull(buffer, data)\
{\
	accquire_spinlock(&(buffer)->bufLock, 0);\
	if ((buffer)->count < (buffer)->capacity) {\
		(buffer)->dataBuf[(buffer)->writeIndex] = (data);\
		(buffer)->writeIndex = ((buffer)->writeIndex + 1) % (buffer)->capacity;\
		(buffer)->count++;\
		if ((buffer)->count > (buffer)->capacity) {\
			(buffer)->count = (buffer)->capacity;\
		}\
	}\
	release_spinlock(&(buffer)->bufLock);\
}

#define BufCount(buffer, outCount)\
{\
	accquire_spinlock(&(buffer)->bufLock, 0);\
	*(outCount) = (buffer)->count;\
	release_spinlock(&cbuf->bufLock);\
}

#endif
