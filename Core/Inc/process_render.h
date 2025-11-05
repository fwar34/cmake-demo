/*
 * process_render.h
 *
 *  Created on: Mar 23, 2025
 *      Author: fwar3
 */

#ifndef INC_PROCESS_RENDER_H_
#define INC_PROCESS_RENDER_H_

#include <inttypes.h>

void ProcessRender();
void ImageMoveToggle();
void ImageMoveLeft(uint8_t pixel);
void ImageMoveRight(uint8_t pixel);

#endif /* INC_PROCESS_RENDER_H_ */
