/*
 * protocol_error.h
 *
 *  Created on: Mar 22, 2025
 *      Author: fwar3
 */
#ifndef __ERROR_DEFINE_H_
#define __ERROR_DEFINE_H_

typedef enum {
    RETURN_VALUE_OK,
    RETURN_VALUE_ERROR,
    RETURN_VALUE_PARAM_INVALID,
} ReturnValue;

typedef enum {
    STORAGE_ERROR_OK,
    STORAGE_ERROR_FAILED,
    STORAGE_ERROR_NOT_FOUND,
    STORAGE_ERROR_FULL,
    STORAGE_ERROR_INVALID_ID,
} StorageError;

typedef enum {
    PROTOCOL_ERROR_OK = 0,
    PROTOCOL_ERROR_INVALID_PARAM,
} ProtocolError;

#endif // !__ERROR_DEFINE_H_
