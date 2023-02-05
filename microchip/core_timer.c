// https://www.microchip.com/forums/m883894.aspx

void __attribute__((vector(_CORE_TIMER_VECTOR), interrupt(CORE_TIMER_INT_LINKAGE), nomips16)) RtcIsr(void)
{
 int ticks;
 
 CPU_ENTERED_ISR();
 
 _serviceRTCInterrupt(); // do whatever needs doing
// Now adjust the core timer match register ("COMPARE"), accounting for possible missed interrupts (due to flash stalls, etc.)
 // Note that Core Timer is allowed to free run and wrap. 
 // We keep moving the Compare register ahead of it on each ISR. Normally we increment it by CORE_TIMER_INTERRUPT_TICKS,
 // but if we missed some interrupts, then it may be incremented by some integer multiple of that.
 
 // Below, we calculate:
 // 1. Ticks past compare value: ReadCT()-MIPSCompareValue (normally small - just interrupt latency)
 // 2. Add half a interrupt interval and divide by in the interrupt interval: +CORE_TIMER_INTERRUPT_TICKS/2) / (signed) CORE_TIMER_INTERRUPT_TICKS
 // That gives the closest whole number of interrupt periods missed (normally zero).
 // 3. Add 1 (for the next interrupt): +1
 // 4. Multiply by CORE_TIMER_INTERRUPT_TICKS to get ticks. This is the offset to the last setting to be added to Count and _RTC64.
 // It accounts for any missed interrupts, and allows at least 1/2 interrupt period before the next match.
 
 ticks = ((ReadCT()-ReadCTCompare()+CORE_TIMER_INTERRUPT_TICKS/2) / (signed) CORE_TIMER_INTERRUPT_TICKS + 1) \
 * CORE_TIMER_INTERRUPT_TICKS;
_rtc64 += ticks;
 
 WriteCTCompare(ReadCTCompare()+ticks);
 
 ClearInterruptFlag(INT_SOURCE_TIMER_CORE);
 
 CPU_EXITED_ISR();
}
