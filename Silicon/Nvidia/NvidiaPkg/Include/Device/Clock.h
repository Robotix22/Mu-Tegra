/*
 * Copyright (c) 2010-2019, NVIDIA CORPORATION.  All rights reserved.
 *
 * SPDX-License-Identifier: BSD-2-Clause-Patent
 */
#ifndef _CLOCK_H_
#define _CLOCK_H_

// Set of oscillator frequencies supported in the internal API.
enum ClockOscFreq {
  // All in MHz, so 13_0 is 13.0MHz
  CLOCK_OSC_FREQ_13_0 = 0,
  CLOCK_OSC_FREQ_16_8,
  CLOCK_OSC_FREQ_19_2 = 4,
  CLOCK_OSC_FREQ_38_4,
  CLOCK_OSC_FREQ_12_0 = 8,
  CLOCK_OSC_FREQ_48_0,
  CLOCK_OSC_FREQ_26_0 = 12,

  CLOCK_OSC_FREQ_COUNT,
};

//
// The oscillator frequency is fixed to one of seven set values. Based on this
// the other clocks are set up appropriately.
//
static unsigned OscFreq[CLOCK_OSC_FREQ_COUNT] = {
  13000000,
  16800000,
  0,
  0,
  19200000,
  38400000,
  0,
  0,
  12000000,
  48000000,
  0,
  0,
  26000000,
};

// PLL registers - there are several PLLs in the clock controller
struct clk_pll {
  UINTN pll_base;                                         // the control register

  // pll_out[0] is output A control, pll_out[1] is output B control
  UINTN pll_out[2];
  UINTN pll_misc;                                         // other misc things
};

// PLL registers - there are several PLLs in the clock controller
struct clk_pll_simple {
  UINTN pll_base;                                         // the control register
  UINTN pll_misc;                                         // other misc things
};

// RST_DEV_(L,H,U,V,W)_(SET,CLR) and CLK_ENB_(L,H,U,V,W)_(SET,CLR)
struct clk_set_clr {
  UINTN set;
  UINTN clr;
};

//
// Most PLLs use the clk_pll structure, but some have a simpler two-member
// structure for which we use clk_pll_simple. The reason for this non-
// othogonal setup is not stated.
//
enum {
  TEGRA_CLK_PLLS        = 6,                              // Number of normal PLLs
  TEGRA_CLK_SIMPLE_PLLS = 3,                              // Number of simple PLLs
  TEGRA_CLK_REGS        = 3,                              // Number of clock enable regs L/H/U
  TEGRA_CLK_SOURCES     = 64,                             // Number of ppl clock sources L/H/U
  TEGRA_CLK_REGS_VW     = 2,                              // Number of clock enable regs V/W
  TEGRA_CLK_SOURCES_VW  = 32,                             // Number of ppl clock sources V/W
  TEGRA_CLK_SOURCES_X   = 32,                             // Number of ppl clock sources X
  TEGRA_CLK_SOURCES_Y   = 18,                             // Number of ppl clock sources Y
};

// Clock/Reset Controller (CLK_RST_CONTROLLER_) regs
struct clk_rst_ctlr {
  UINTN crc_rst_src;                                      // _RST_SOURCE_0, 0x00
  UINTN crc_rst_dev[TEGRA_CLK_REGS];                      // _RST_DEVICES_L/H/U_0
  UINTN crc_clk_out_enb[TEGRA_CLK_REGS];                  // _CLK_OUT_ENB_L/H/U_0
  UINTN crc_reserved0;                                    // reserved_0, 0x1C
  UINTN crc_cclk_brst_pol;                                // _CCLK_BURST_POLICY_0, 0x20
  UINTN crc_super_cclk_div;                               // _SUPER_CCLK_DIVIDER_0, 0x24
  UINTN crc_sclk_brst_pol;                                // _SCLK_BURST_POLICY_0, 0x28
  UINTN crc_super_sclk_div;                               // _SUPER_SCLK_DIVIDER_0, 0x2C
  UINTN crc_clk_sys_rate;                                 // _CLK_SYSTEM_RATE_0, 0x30
  UINTN crc_prog_dly_clk;                                 // _PROG_DLY_CLK_0, 0x34
  UINTN crc_aud_sync_clk_rate;                            // _AUDIO_SYNC_CLK_RATE_0, 0x38
  UINTN crc_reserved1;                                    // reserved_1, 0x3C
  UINTN crc_cop_clk_skip_plcy;                            // _COP_CLK_SKIP_POLICY_0, 0x40
  UINTN crc_clk_mask_arm;                                 // _CLK_MASK_ARM_0, 0x44
  UINTN crc_misc_clk_enb;                                 // _MISC_CLK_ENB_0, 0x48
  UINTN crc_clk_cpu_cmplx;                                // _CLK_CPU_CMPLX_0, 0x4C
  UINTN crc_osc_ctrl;                                     // _OSC_CTRL_0, 0x50
  UINTN crc_pll_lfsr;                                     // _PLL_LFSR_0, 0x54
  UINTN crc_osc_freq_det;                                 // _OSC_FREQ_DET_0, 0x58
  UINTN crc_osc_freq_det_stat;                            // _OSC_FREQ_DET_STATUS_0, 0x5C
  UINTN crc_reserved2[8];                                 // reserved_2[8], 0x60-7C
  struct clk_pll crc_pll[TEGRA_CLK_PLLS];                 // PLLs from 0x80 to 0xdc

  // PLLs from 0xe0 to 0xf4
  struct clk_pll_simple crc_pll_simple[TEGRA_CLK_SIMPLE_PLLS];
  UINTN crc_reserved10;                                   // _reserved_10, 0xF8
  UINTN crc_reserved11;                                   // _reserved_11, 0xFC
  UINTN crc_clk_src[TEGRA_CLK_SOURCES];                   //_I2S1_0... 0x100-1fc
  UINTN crc_reserved20[32];                               // _reserved_20, 0x200-27c
  UINTN crc_clk_out_enb_x;                                // _CLK_OUT_ENB_X_0, 0x280
  UINTN crc_clk_enb_x_set;                                // _CLK_ENB_X_SET_0, 0x284
  UINTN crc_clk_enb_x_clr;                                // _CLK_ENB_X_CLR_0, 0x288
  UINTN crc_rst_devices_x;                                // _RST_DEVICES_X_0, 0x28c
  UINTN crc_rst_dev_x_set;                                // _RST_DEV_X_SET_0, 0x290
  UINTN crc_rst_dev_x_clr;                                // _RST_DEV_X_CLR_0, 0x294
  UINTN crc_clk_out_enb_y;                                // _CLK_OUT_ENB_Y_0, 0x298
  UINTN crc_clk_enb_y_set;                                // _CLK_ENB_Y_SET_0, 0x29c
  UINTN crc_clk_enb_y_clr;                                // _CLK_ENB_Y_CLR_0, 0x2a0
  UINTN crc_rst_devices_y;                                // _RST_DEVICES_Y_0, 0x2a4
  UINTN crc_rst_dev_y_set;                                // _RST_DEV_Y_SET_0, 0x2a8
  UINTN crc_rst_dev_y_clr;                                // _RST_DEV_Y_CLR_0, 0x2ac
  UINTN crc_reserved21[17];                               // _reserved_21, 0x2b0-2f0
  UINTN crc_dfll_base;                                    // _DFLL_BASE_0, 0x2f4
  UINTN crc_reserved22[2];                                // _reserved_22, 0x2f8-2fc

  // _RST_DEV_L/H/U_SET_0 0x300 ~ 0x314
  struct clk_set_clr crc_rst_dev_ex[TEGRA_CLK_REGS];
  UINTN crc_reserved30[2];                                // _reserved_30, 0x318, 0x31c

  // _CLK_ENB_L/H/U_CLR_0 0x320 ~ 0x334
  struct clk_set_clr crc_clk_enb_ex[TEGRA_CLK_REGS];
  UINTN crc_reserved31[2];                                // _reserved_31, 0x338, 0x33c
  UINTN crc_cpu_cmplx_set;                                // _RST_CPU_CMPLX_SET_0, 0x340
  UINTN crc_cpu_cmplx_clr;                                // _RST_CPU_CMPLX_CLR_0, 0x344

  // Additional (T30) registers
  UINTN crc_clk_cpu_cmplx_set;                            // _CLK_CPU_CMPLX_SET_0, 0x348
  UINTN crc_clk_cpu_cmplx_clr;                            // _CLK_CPU_CMPLX_SET_0, 0x34c
  UINTN crc_reserved32[2];                                // _reserved_32, 0x350, 0x354
  UINTN crc_rst_dev_vw[TEGRA_CLK_REGS_VW];                // _RST_DEVICES_V/W_0
  UINTN crc_clk_out_enb_vw[TEGRA_CLK_REGS_VW];            // _CLK_OUT_ENB_V/W_0
  UINTN crc_cclkg_brst_pol;                               // _CCLKG_BURST_POLICY_0, 0x368
  UINTN crc_super_cclkg_div;                              // _SUPER_CCLKG_DIVIDER_0, 0x36C
  UINTN crc_cclklp_brst_pol;                              // _CCLKLP_BURST_POLICY_0, 0x370
  UINTN crc_super_cclkp_div;                              // _SUPER_CCLKLP_DIVIDER_0, 0x374
  UINTN crc_clk_cpug_cmplx;                               // _CLK_CPUG_CMPLX_0, 0x378
  UINTN crc_clk_cpulp_cmplx;                              // _CLK_CPULP_CMPLX_0, 0x37C
  UINTN crc_cpu_softrst_ctrl;                             // _CPU_SOFTRST_CTRL_0, 0x380
  UINTN crc_cpu_softrst_ctrl1;                            // _CPU_SOFTRST_CTRL1_0, 0x384
  UINTN crc_cpu_softrst_ctrl2;                            // _CPU_SOFTRST_CTRL2_0, 0x388
  UINTN crc_reserved33[9];                                // _reserved_33, 0x38c-3ac
  UINTN crc_clk_src_vw[TEGRA_CLK_SOURCES_VW];             // 0x3B0-0x42C

  // _RST_DEV_V/W_SET_0 0x430 ~ 0x43c
  struct clk_set_clr crc_rst_dev_ex_vw[TEGRA_CLK_REGS_VW];

  // _CLK_ENB_V/W_CLR_0 0x440 ~ 0x44c
  struct clk_set_clr crc_clk_enb_ex_vw[TEGRA_CLK_REGS_VW];

  // Additional (T114+) registers
  UINTN crc_rst_cpug_cmplx_set;                           // _RST_CPUG_CMPLX_SET_0, 0x450
  UINTN crc_rst_cpug_cmplx_clr;                           // _RST_CPUG_CMPLX_CLR_0, 0x454
  UINTN crc_rst_cpulp_cmplx_set;                          // _RST_CPULP_CMPLX_SET_0, 0x458
  UINTN crc_rst_cpulp_cmplx_clr;                          // _RST_CPULP_CMPLX_CLR_0, 0x45C
  UINTN crc_clk_cpug_cmplx_set;                           // _CLK_CPUG_CMPLX_SET_0, 0x460
  UINTN crc_clk_cpug_cmplx_clr;                           // _CLK_CPUG_CMPLX_CLR_0, 0x464
  UINTN crc_clk_cpulp_cmplx_set;                          // _CLK_CPULP_CMPLX_SET_0, 0x468
  UINTN crc_clk_cpulp_cmplx_clr;                          // _CLK_CPULP_CMPLX_CLR_0, 0x46C
  UINTN crc_cpu_cmplx_status;                             // _CPU_CMPLX_STATUS_0, 0x470
  UINTN crc_reserved40[1];                                // _reserved_40, 0x474
  UINTN crc_intstatus;                                    // __INTSTATUS_0, 0x478
  UINTN crc_intmask;                                      // __INTMASK_0, 0x47C
  UINTN crc_utmip_pll_cfg0;                               // _UTMIP_PLL_CFG0_0, 0x480
  UINTN crc_utmip_pll_cfg1;                               // _UTMIP_PLL_CFG1_0, 0x484
  UINTN crc_utmip_pll_cfg2;                               // _UTMIP_PLL_CFG2_0, 0x488
  UINTN crc_plle_aux;                                     // _PLLE_AUX_0, 0x48C
  UINTN crc_sata_pll_cfg0;                                // _SATA_PLL_CFG0_0, 0x490
  UINTN crc_sata_pll_cfg1;                                // _SATA_PLL_CFG1_0, 0x494
  UINTN crc_pcie_pll_cfg0;                                // _PCIE_PLL_CFG0_0, 0x498
  UINTN crc_prog_audio_dly_clk;                           // _PROG_AUDIO_DLY_CLK_0, 0x49C
  UINTN crc_audio_sync_clk_i2s0;                          // _AUDIO_SYNC_CLK_I2S0_0, 0x4A0
  UINTN crc_audio_sync_clk_i2s1;                          // _AUDIO_SYNC_CLK_I2S1_0, 0x4A4
  UINTN crc_audio_sync_clk_i2s2;                          // _AUDIO_SYNC_CLK_I2S2_0, 0x4A8
  UINTN crc_audio_sync_clk_i2s3;                          // _AUDIO_SYNC_CLK_I2S3_0, 0x4AC
  UINTN crc_audio_sync_clk_i2s4;                          // _AUDIO_SYNC_CLK_I2S4_0, 0x4B0
  UINTN crc_audio_sync_clk_spdif;                         // _AUDIO_SYNC_CLK_SPDIF_0, 0x4B4
  UINTN crc_plld2_base;                                   // _PLLD2_BASE_0, 0x4B8
  UINTN crc_plld2_misc;                                   // _PLLD2_MISC_0, 0x4BC
  UINTN crc_utmip_pll_cfg3;                               // _UTMIP_PLL_CFG3_0, 0x4C0
  UINTN crc_pllrefe_base;                                 // _PLLREFE_BASE_0, 0x4C4
  UINTN crc_pllrefe_misc;                                 // _PLLREFE_MISC_0, 0x4C8
  UINTN crs_reserved_50[7];                               // _reserved_50, 0x4CC-0x4E4
  UINTN crc_pllc2_base;                                   // _PLLC2_BASE_0, 0x4E8
  UINTN crc_pllc2_misc0;                                  // _PLLC2_MISC_0_0, 0x4EC
  UINTN crc_pllc2_misc1;                                  // _PLLC2_MISC_1_0, 0x4F0
  UINTN crc_pllc2_misc2;                                  // _PLLC2_MISC_2_0, 0x4F4
  UINTN crc_pllc2_misc3;                                  // _PLLC2_MISC_3_0, 0x4F8
  UINTN crc_pllc3_base;                                   // _PLLC3_BASE_0, 0x4FC
  UINTN crc_pllc3_misc0;                                  // _PLLC3_MISC_0_0, 0x500
  UINTN crc_pllc3_misc1;                                  // _PLLC3_MISC_1_0, 0x504
  UINTN crc_pllc3_misc2;                                  // _PLLC3_MISC_2_0, 0x508
  UINTN crc_pllc3_misc3;                                  // _PLLC3_MISC_3_0, 0x50C
  UINTN crc_pllx_misc1;                                   // _PLLX_MISC_1_0, 0x510
  UINTN crc_pllx_misc2;                                   // _PLLX_MISC_2_0, 0x514
  UINTN crc_pllx_misc3;                                   // _PLLX_MISC_3_0, 0x518
  UINTN crc_xusbio_pll_cfg0;                              // _XUSBIO_PLL_CFG0_0, 0x51C
  UINTN crc_xusbio_pll_cfg1;                              // _XUSBIO_PLL_CFG0_1, 0x520
  UINTN crc_plle_aux1;                                    // _PLLE_AUX1_0, 0x524
  UINTN crc_pllp_reshift;                                 // _PLLP_RESHIFT_0, 0x528
  UINTN crc_utmipll_hw_pwrdn_cfg0;                        // _UTMIPLL_HW_PWRDN_CFG0_0, 0x52C
  UINTN crc_pllu_hw_pwrdn_cfg0;                           // _PLLU_HW_PWRDN_CFG0_0, 0x530
  UINTN crc_xusb_pll_cfg0;                                // _XUSB_PLL_CFG0_0, 0x534
  UINTN crc_reserved51[1];                                // _reserved_51, 0x538
  UINTN crc_clk_cpu_misc;                                 // _CLK_CPU_MISC_0, 0x53C
  UINTN crc_clk_cpug_misc;                                // _CLK_CPUG_MISC_0, 0x540
  UINTN crc_clk_cpulp_misc;                               // _CLK_CPULP_MISC_0, 0x544
  UINTN crc_pllx_hw_ctrl_cfg;                             // _PLLX_HW_CTRL_CFG_0, 0x548
  UINTN crc_pllx_sw_ramp_cfg;                             // _PLLX_SW_RAMP_CFG_0, 0x54C
  UINTN crc_pllx_hw_ctrl_status;                          // _PLLX_HW_CTRL_STATUS_0, 0x550
  UINTN crc_reserved52[1];                                // _reserved_52, 0x554
  UINTN crc_super_gr3d_clk_div;                           // _SUPER_GR3D_CLK_DIVIDER_0, 0x558
  UINTN crc_spare_reg0;                                   // _SPARE_REG0_0, 0x55C
  UINT32 _rsv32[4];                                       // 0x560-0x56c
  UINT32 crc_plld2_ss_cfg;                                // _PLLD2_SS_CFG, 0x570
  UINT32 _rsv32_1[7];                                     // 0x574-58c
  struct clk_pll_simple plldp;                            // _PLLDP_BASE, 0x590 _PLLDP_MISC
  UINT32 crc_plldp_ss_cfg;                                // _PLLDP_SS_CFG, 0x598

  // Tegra124+ - skip to 0x600 here for new CLK_SOURCE_ regs
  UINTN _rsrv32_2[25];                                    // _0x59C - 0x5FC
  UINTN crc_clk_src_x[TEGRA_CLK_SOURCES_X];               // XUSB, etc, 0x600-0x67C

  // Tegra210 - skip to 0x694 here for new CLK_SOURCE_ regs
  UINTN crc_reserved61[5];                                // _reserved_61, 0x680 - 0x690
  //
  // NOTE: PLLA1 regs are in the middle of this Y region. Break this in
  // two later if PLLA1 is needed, but for now this is cleaner.
  //
  UINTN crc_clk_src_y[TEGRA_CLK_SOURCES_Y];               // SPARE1, etc, 0x694-0x6D8
};

#endif /* _CLOCK_H_ */