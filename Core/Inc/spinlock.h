#ifndef __LOCK_H_
#define __LOCK_H_

#include <inttypes.h>
#include <stdbool.h>

#define SpinLock int32_t

bool try_accquire_lock(volatile int32_t *lock);
bool accquire_spinlock(volatile int32_t *lock, uint32_t ms);
void release_spinlock(volatile int32_t *lock);

#endif
