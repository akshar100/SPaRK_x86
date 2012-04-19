#include <rtl_conf.h>
#include <rtl_sched.h>
#include <rtl_core.h>

#ifdef CONFIG_RTL_FP_SUPPORT
extern inline void rtl_fpu_save (schedule_t *s, RTL_THREAD_STRUCT *current_t)
{
	__asm__("fnsave %0\n\tfwait" : "=m" (current_t->fpu_regs));
}


extern inline void rtl_fpu_restore (schedule_t *s,RTL_THREAD_STRUCT *current_t) {
	__asm__("clts\n"); /* just in case */
	if (current_t->fpu_initialized) {
		__asm__("frstor %0" : "=m" (current_t->fpu_regs));
	} else {
		__asm__("fninit");
		current_t->fpu_initialized = 1;
	}

}


extern inline void rtl_task_init_fpu (RTL_THREAD_STRUCT *t, RTL_THREAD_STRUCT *fpu_owner)
{
}

#endif /* CONFIG_RTL_FP_SUPPORT */

