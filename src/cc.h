#ifndef F29H85_SOES_CC_WRAPPER_H
#define F29H85_SOES_CC_WRAPPER_H

#include_next <cc.h>

#ifndef PRINTLN
#define PRINTLN(fmt, ...) do { printf(fmt, ##__VA_ARGS__); printf("\n"); } while (0)
#endif

#endif
