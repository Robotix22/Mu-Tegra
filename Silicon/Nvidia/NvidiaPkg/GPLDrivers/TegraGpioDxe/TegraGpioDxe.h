/** @file
  Copyright (C) 2013-2015 NVIDIA Corporation <www.nvidia.com>

  SPDX-License-Identifier: GPL-2.0+
**/

#ifndef _TEGRA_GPIO_DXE_H_
#define _TEGRA_GPIO_DXE_H_

//
// The Tegra GPIO Controller has 256 GPIOs in 8 Banks of 4 Ports,
// each with 8 GPIOs.
//
#define TEGRA_GPIO_PORTS   4
#define TEGRA_GPIO_BANKS   8

#define GPIO_BANK(x)      ((x) >> 5)
#define GPIO_PORT(x)      (((x) >> 3) & 0x3)
#define GPIO_FULLPORT(x)  ((x) >> 3)
#define GPIO_BIT(x)       ((x) & 0x7)

// GPIO Controller Registers for a Single Bank
typedef struct gpio_ctlr_bank {
  UINT32 gpio_config[TEGRA_GPIO_PORTS];
  UINT32 gpio_dir_out[TEGRA_GPIO_PORTS];
  UINT32 gpio_out[TEGRA_GPIO_PORTS];
  UINT32 gpio_in[TEGRA_GPIO_PORTS];
  UINT32 gpio_int_status[TEGRA_GPIO_PORTS];
  UINT32 gpio_int_enable[TEGRA_GPIO_PORTS];
  UINT32 gpio_int_level[TEGRA_GPIO_PORTS];
  UINT32 gpio_int_clear[TEGRA_GPIO_PORTS];
  UINT32 gpio_masked_config[TEGRA_GPIO_PORTS];
  UINT32 gpio_masked_dir_out[TEGRA_GPIO_PORTS];
  UINT32 gpio_masked_out[TEGRA_GPIO_PORTS];
  UINT32 gpio_masked_in[TEGRA_GPIO_PORTS];
  UINT32 gpio_masked_int_status[TEGRA_GPIO_PORTS];
  UINT32 gpio_masked_int_enable[TEGRA_GPIO_PORTS];
  UINT32 gpio_masked_int_level[TEGRA_GPIO_PORTS];
  UINT32 gpio_masked_int_clear[TEGRA_GPIO_PORTS];
} GpioCtrlBank;

typedef struct gpio_ctlr {
  struct gpio_ctlr_bank gpio_bank[TEGRA_GPIO_BANKS];
} GpioCtrl;

#endif /* _TEGRA_GPIO_DXE_H_ */
