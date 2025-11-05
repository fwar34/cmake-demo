#include "spinlock.h"
#include "stm32f1xx_hal.h"

/*
 * ptr：指向需要进行CAS操作的变量的指针。
 * old：期望的旧值。
 * new：如果ptr指向的值等于old，则将ptr指向的值更新为new。
 * 返回值：如果CAS操作成功（即ptr指向的值等于old并被更新为new），返回1；否则返回0。
 */
static int cas(volatile int32_t *ptr, int32_t old, int32_t new) {
    int temp;
#if 0
    __asm__ __volatile__ (
        "LDREX %0, [%1]\n\t" // 加载目标地址的值到寄存器
        "CMP %0, %2\n\t"     // 比较加载的值与期望的旧值
        "BNE end\n\t"        // 如果不相等，跳转到end标签
        "STREX %0, %3, [%1]\n\t" // 尝试将新值写入目标地址
        "CMP %0, #0\n\t"     // 检查STREX操作是否成功
        "BNE end\n\t"        // 如果不成功，跳转到end标签
        "MOV %0, #1\n\t"     // 如果成功，将返回值设置为1
        "end:"
        : "=&r" (temp)        // 输出操作数，表示返回值
        : "r" (ptr), "r" (old), "r" (new) // 输入操作数
        : "cc"                // 被修改的寄存器
    );
#else
    __asm__ __volatile__(
        "LDREX %0, [%1]\n\t"           // 加载目标地址的值到寄存器
        "CMP %0, %2\n\t"               // 比较加载的值与期望的旧值
        "BNE 1f\n\t"                   // 如果不相等，跳转到标签1（向前引用）
        "STREX %0, %3, [%1]\n\t"       // 尝试将新值写入目标地址
        "CMP %0, #0\n\t"               // 检查STREX操作是否成功
        "BNE 1f\n\t"                   // 如果不成功，跳转到标签1（向前引用）
        "MOV %0, #1\n\t"               // 如果成功，将返回值设置为1
        "1:\n\t"                       // 定义局部标签1
        : "=&r"(temp)                  // 输出操作数，表示返回值
        : "r"(ptr), "r"(old), "r"(new) // 输入操作数
        : "cc"                         // 被修改的寄存器
    );
#endif
    return temp;
}

bool try_accquire_lock(volatile int32_t *lock)
{
	return cas(lock, 0, 1);
}
bool accquire_spinlock(volatile int32_t *lock, uint32_t ms)
{
	uint32_t beginTick = HAL_GetTick();
	while (!try_accquire_lock(lock)) {
//		HAL_Delay(1);
		if (ms != 0 && HAL_GetTick() - beginTick >= ms * 1000) {
			return false;
		}
	}
	return true;
}
void release_spinlock(volatile int32_t *lock)
{
	cas(lock, 1, 0);
}
