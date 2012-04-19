/*
 * mprot.h
 *
 * Written by Vicente Esteve LLoret <viesllo@inf.upv.es>
 * Copyright (C) Jul, 2003 OCERA Consortium.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation version 2.
 *
 * memory protection.
 *
 */


#ifndef _MPROT_H_
#define _MPROT_H_

#include <rtl_conf.h>
#include <arch/page.h>

#if 0

#define STARTKERNELCODE(v) CLEAR_WP(v)  
#define ENDKERNELCODE(v)   RESTORE_WP(v)
#define STARTUSERCODE()    SET_WP_ON()
#define ENDUSERCODE()      SET_WP_OFF()

#endif // CONFIG_KERNEL_MEMORYPROT

#define STARTKERNELCODE(v)
#define ENDKERNELCODE(v)
#define STARTUSERCODE()
#define ENDUSERCODE()



#endif //_MPROT_H_
