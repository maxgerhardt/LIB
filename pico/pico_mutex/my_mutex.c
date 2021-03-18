/*

    The change does not fail old source codes

    Note:
        string 'mutex_' is replaced to 'my_mutex_'

*/


/*
 * Copyright (c) 2020 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include "my_mutex.h"
#include "pico/time.h"

#if !PICO_NO_HARDWARE
static_assert(sizeof(my_mutex_t) == 8, ""); /* no change, uint16_t counter, one byte up to 8 remain */
#endif

void my_mutex_init(my_mutex_t *mtx) {
    lock_init(&mtx->core, next_striped_spin_lock_num());
    mtx->owner = -1;
    mtx->counter = 0; //<------------------- NEW - init counter
    __mem_fence_release();
}

void __time_critical_func(my_mutex_enter_blocking)(my_mutex_t *mtx) {
    assert(mtx->core.spin_lock);
    bool block = true;
    do {
        uint32_t save = spin_lock_blocking(mtx->core.spin_lock);
        if (mtx->owner < 0) {
            mtx->owner = (int8_t)get_core_num();
            block = false;
        }
        spin_unlock(mtx->core.spin_lock, save);
        if (block) {
            __wfe();
        }
    } while (block);
}

bool __time_critical_func(my_mutex_try_enter)(my_mutex_t *mtx, uint32_t *owner_out) {
    bool entered;
    uint32_t save = spin_lock_blocking(mtx->core.spin_lock);
    if (mtx->owner < 0) {
        mtx->owner = (int8_t)get_core_num();
        entered = true;
    } else {
        if (owner_out) *owner_out = (uint32_t) mtx->owner;
        entered = false;
    }
    spin_unlock(mtx->core.spin_lock, save);
    return entered;
}

bool __time_critical_func(my_mutex_enter_timeout_ms)(my_mutex_t *mtx, uint32_t timeout_ms) {
    return my_mutex_enter_block_until(mtx, make_timeout_time_ms(timeout_ms));
}

bool __time_critical_func(my_mutex_enter_block_until)(my_mutex_t *mtx, absolute_time_t until) {
    assert(mtx->core.spin_lock);
    bool block = true;
    do {
        uint32_t save = spin_lock_blocking(mtx->core.spin_lock);
        if (mtx->owner < 0) {
            mtx->owner = (int8_t)get_core_num();
            block = false;
        }
        spin_unlock(mtx->core.spin_lock, save);
        if (block) {
            if (best_effort_wfe_or_timeout(until)) {
                return false;
            }
        }
    } while (block);
    return true;
}

void __time_critical_func(my_mutex_exit)(my_mutex_t *mtx) {
    uint32_t save = spin_lock_blocking(mtx->core.spin_lock);
    assert(mtx->owner >= 0);
    mtx->owner = -1;
    mtx->counter = 0; //<------------------- NEW - init counter 
    __sev();
    spin_unlock(mtx->core.spin_lock, save);
}

/*

    New functions

*/

void my_mutex_init_recursive(my_mutex_t *mtx) __attribute__((alias("my_mutex_init")));

// GIVE
void __time_critical_func(my_mutex_exit_recursive)(my_mutex_t *mtx)
{
    if (mtx->owner = (int8_t)get_core_num())
    {
        if (--mtx->counter == 0)
        {
            my_mutex_exit(mtx);
        }
    } // else the core is not owner
}

// TAKE
void __time_critical_func(my_mutex_enter_blocking_recursive)(my_mutex_t *mtx)
{
    if (mtx->owner = (int8_t)get_core_num())
    {
        ++mtx->counter;
    }
    else
    {
        my_mutex_enter_blocking(mtx);
    }
}

int __time_critical_func(my_mutex_try_enter_recursive)(my_mutex_t *mtx)
{
    if (mtx->owner == (int8_t)get_core_num())
    {
        ++mtx->counter;
        return 1; // pdPASS -> pdTRUE -> 1
    }

    bool xReturn = my_mutex_try_enter(mtx, NULL); // FIX? return ... true = unowned,

    /* 
     * xReturn = xQueueSemaphoreTake( pxMutex, 0 );
     * pdPASS(1) will only be returned if the mutex was successfully obtained.  
     * The calling task may have entered the Blocked state before reaching here. 
     * https://github.com/FreeRTOS/FreeRTOS-Kernel/blob/23f641850d2428eac3e164d6e735e6e92dc3914a/queue.c#L691
     * 
     */

    if (xReturn != false) // FIX? ... if ( xReturn != pdFAIL(0) )
    {
        ++mtx->counter;
    }

    return xReturn; // as newlib <sys/lock.h>
}