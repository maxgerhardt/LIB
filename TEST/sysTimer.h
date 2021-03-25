
#ifndef _SYS_TIMER_H_
#define _SYS_TIMER_H_

#include <stdint.h>
#include <stdbool.h>

typedef enum SYS_TimerMode_t
{
  SYS_TIMER_INTERVAL_MODE,
  SYS_TIMER_PERIODIC_MODE,
} SYS_TimerMode_t;

typedef struct SYS_Timer_t
{
  struct SYS_Timer_t *next;
  uint32_t timeout;

  uint32_t interval;
  SYS_TimerMode_t mode;
  void (*handler)(struct SYS_Timer_t *timer);
} SYS_Timer_t;

void SYS_TimerInit(void);
void SYS_TimerStart(SYS_Timer_t *timer);
void SYS_TimerStop(SYS_Timer_t *timer);
bool SYS_TimerStarted(SYS_Timer_t *timer);
void SYS_TimerTaskHandler(void);

#endif // _SYS_TIMER_H_
