// #pragma once

#ifndef _MOD_MAIN_H_
#define _MOD_MAIN_H_

#include "mod/mod_func.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>


extern void mod_init(float carSizeW, float carSizeH, int w, int h);

extern void reset_modflag(void);

extern void get_modDetectSW(int * modDetectSWMain);

extern void set_mod_start(can_bus_info_t * canbus);

#endif