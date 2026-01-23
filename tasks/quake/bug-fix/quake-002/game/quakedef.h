/*
 * quakedef.h - Minimal definitions for zone allocator testing
 */

#ifndef QUAKEDEF_H
#define QUAKEDEF_H

#include <limits.h>

typedef unsigned char byte;

/* These are provided by the test harness */
extern void Sys_Error(const char *fmt, ...);
extern void Con_Printf(const char *fmt, ...);
extern void Q_memset(void *dest, int fill, int count);

#endif /* QUAKEDEF_H */
