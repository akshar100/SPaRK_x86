#ifndef __RTL_I386_CONSTANTS_H__
#define __RTL_I386_CONSTANTS_H__

#define MACHDEPREGS struct pt_regs 
typedef void intercept_t; /*intercept returns nothing */
#define MACHDEPREGS_PTR(x) (&x)
#define POS_TO_BIT(x) (1UL << x)
#define ARCH_DEFINED_DISABLE 0
#define ARCH_DEFINED_ENABLE 0x200

#define IRQ_MAX_COUNT 256

#ifndef RTL_NR_CPUS
#define RTL_NR_CPUS 1
#endif

#define rtl_getcpuid() 1


/* TODO  in rtl_intercept and rtl_local intercept there is no
   need to have 3 copies of restore_all. The code should 
   simply goto a single block.

   Also the ret_from _intr should be used in rtl_intercept as well.
   No need to do the stupid return. And this address should be patched
   into the code instead of being indirect jumped 
   */

#endif  /* __RTL_I386_CONSTANTS_H__ */
