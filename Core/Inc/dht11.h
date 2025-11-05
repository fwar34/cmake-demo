/*
 * dht11.h
 *
 *  Created on: Mar 8, 2025
 *      Author: fwar3
 */

#ifndef INC_DHT11_H_
#define INC_DHT11_H_

#include <inttypes.h>
#include <stdbool.h>

typedef struct
{
	float temperature;
	float humidity;
} Dht11Result;

void Dht11Init();
void Dht11Process();
void GetDht11Result(Dht11Result *result);

#endif /* INC_DHT11_H_ */
