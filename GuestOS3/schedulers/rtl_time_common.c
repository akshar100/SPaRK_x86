/*
 */
#include <rtl_conf.h>
#include <rtl_time.h>
#include <hypercall.h>
#include <arch/timer_handler.h>

//#define CLOCK_TICK_RATE 200 // Timer tick every 5 ms
#define CLOCK_TICK_RATE 500 // Timer tick every 2 ms
void timer_irq();
struct rtl_clock _i8254_clock;

TIMER_IRQ()

/*******************************************************************/
hrtime_t gethrtime(void)
{
	return _i8254_clock.value;
}
/*******************************************************************/
void timer_irq_handler()
{
	_i8254_clock.value += _i8254_clock.resolution;
	// spark_print("Guest 1:Inside rtl_schedule\n");
	rtl_schedule();
	return;
}
/*******************************************************************/
int init_clocks ()
{
	_i8254_clock.value = 0;
	_i8254_clock.resolution = HRTICKS_PER_SEC / CLOCK_TICK_RATE;
	//spark_registerTimer(_i8254_clock.resolution , timer_irq_handler);
	spark_registerTimer(_i8254_clock.resolution , timer_irq, &_i8254_clock.value);  // see TIMER_IRQ ..control reached timer_irq_handler
	return 0;
}
/*******************************************************************/
