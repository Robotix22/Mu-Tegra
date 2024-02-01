#ifndef _TEGRA_CLOCK_LIB_H_
#define _TEGRA_CLOCK_LIB_H_

#include <Device/Clock.h>

ClockPllInfo *GetPllInfoTable (enum ClockID ClockID);

enum OscClockFreq GetOscClockFreq (VOID);

#endif /* _TEGRA_CLOCK_LIB_H_ */
