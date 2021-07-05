#include <ultra64.h>
#include <PR/gs2dex.h>
#include "config.h"
#include "init.h"
#include "debug.h"
#include "s2d_error.h"

void s2d_init(void) {
	s2d_error_y = TEX_HEIGHT;
}

void s2d_stop(void) {
	s2d_reset_defer_index();
}

