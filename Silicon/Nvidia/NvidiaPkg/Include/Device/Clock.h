/** @file
  Copyright (c) 2011 The Chromium OS Authors.
  Copyright (C) 2010-2014 NVIDIA Corporation <www.nvidia.com>

  SPDX-License-Identifier: GPL-2.0+
**/

#ifndef _CLOCK_H_
#define _CLOCK_H_

// PLL Registers - There are several PLLs in the Clock Controller
typedef struct clk_pll {
  UINT32 PllBase;                                                 // The Control Register
  UINT32 PllOut[2];                                               // 0 is A Control, 1 is B Control
  UINT32 PllMisc;                                                 // Other Misc Things
} ClockPll;

// PLL Registers - There ware several PLLs in the Clock Controller
typedef struct clk_pll_simple {
  UINT32 PllBase;                                                 // The Control Register
  UINT32 PllMisc;                                                 // Other Misc Things
} SimpleClockPll;

// RST_DEV_(L,H,U,V,W)_(SET,CLR) and CLK_ENB_(L,H,U,V,W)_(SET,CLR)
typedef struct clk_set_clr {
  UINT32 Set;
  UINT32 Clr;
} ClockSetClr;

//
// Most PLLs use the clk_pll structure, but some have a simpler two-member
// structure for which we use clk_pll_simple. The reason for this non-
// othogonal setup is not stated.
//
enum {
  TEGRA_CLK_PLLS        = 6,                                      // Number of Normal PLLs
  TEGRA_CLK_SIMPLE_PLLS = 3,                                      // Number of Simple PLLs
  TEGRA_CLK_REGS        = 3,                                      // Number of Clock Enable Registers L/H/U
  TEGRA_CLK_SOURCES     = 64,                                     // Number of PPL Clock Sources L/H/U
  TEGRA_CLK_REGS_VW     = 2,                                      // Number of Clock Enable Registers V/W
  TEGRA_CLK_SOURCES_VW  = 32,                                     // Number of PPL Clock Sources V/W
  TEGRA_CLK_SOURCES_X   = 32,                                     // Number of PPL Clock Sources X
  TEGRA_CLK_SOURCES_Y   = 18                                      // Number of PPL Clock Sources Y
};

// Clock Reset Controller (CLK_RST_CONTROLLER_) Registers
typedef struct clk_rst_ctlr {
  UINT32 crc_rst_src;                                             // _RST_SOURCE_0,             0x00
  UINT32 crc_rst_dev[TEGRA_CLK_REGS];                             // _RST_DEVICES_L/H/U_0
  UINT32 crc_clk_out_enb[TEGRA_CLK_REGS];                         // _CLK_OUT_ENB_L/H/U_0
  UINT32 crc_reserved0;                                           // reserved_0,                0x1C
  UINT32 crc_cclk_brst_pol;                                       // _CCLK_BURST_POLICY_0,      0x20
  UINT32 crc_super_cclk_div;                                      // _SUPER_CCLK_DIVIDER_0,     0x24
  UINT32 crc_sclk_brst_pol;                                       // _SCLK_BURST_POLICY_0,      0x28
  UINT32 crc_super_sclk_div;                                      // _SUPER_SCLK_DIVIDER_0,     0x2C
  UINT32 crc_clk_sys_rate;                                        // _CLK_SYSTEM_RATE_0,        0x30
  UINT32 crc_prog_dly_clk;                                        // _PROG_DLY_CLK_0,           0x34
  UINT32 crc_aud_sync_clk_rate;                                   // _AUDIO_SYNC_CLK_RATE_0,    0x38
  UINT32 crc_reserved1;                                           // reserved_1,                0x3C
  UINT32 crc_cop_clk_skip_plcy;                                   // _COP_CLK_SKIP_POLICY_0,    0x40
  UINT32 crc_clk_mask_arm;                                        // _CLK_MASK_ARM_0,           0x44
  UINT32 crc_misc_clk_enb;                                        // _MISC_CLK_ENB_0,           0x48
  UINT32 crc_clk_cpu_cmplx;                                       // _CLK_CPU_CMPLX_0,          0x4C
  UINT32 crc_osc_ctrl;                                            // _OSC_CTRL_0,               0x50
  UINT32 crc_pll_lfsr;                                            // _PLL_LFSR_0,               0x54
  UINT32 crc_osc_freq_det;                                        // _OSC_FREQ_DET_0,           0x58
  UINT32 crc_osc_freq_det_stat;                                   // _OSC_FREQ_DET_STATUS_0,    0x5C
  UINT32 crc_reserved2[8];                                        // reserved_2[8],             0x60  - 0x7C

  ClockPll crc_pll[TEGRA_CLK_PLLS];                               // PLLs from                  0x80  - 0xdc

  SimpleClockPll crc_pll_simple[TEGRA_CLK_SIMPLE_PLLS];           // PLLs from                  0xe0  - 0xf4

  UINT32 crc_reserved10;                                          // _reserved_10,              0xF8
  UINT32 crc_reserved11;                                          // _reserved_11,              0xFC
  UINT32 crc_clk_src[TEGRA_CLK_SOURCES];                          // _I2S1_0...,                0x100 - 0x1fc
  UINT32 crc_reserved20[32];                                      // _reserved_20,              0x200 - 0x27c
  UINT32 crc_clk_out_enb_x;                                       // _CLK_OUT_ENB_X_0,          0x280
  UINT32 crc_clk_enb_x_set;                                       // _CLK_ENB_X_SET_0,          0x284
  UINT32 crc_clk_enb_x_clr;                                       // _CLK_ENB_X_CLR_0,          0x288
  UINT32 crc_rst_devices_x;                                       // _RST_DEVICES_X_0,          0x28c
  UINT32 crc_rst_dev_x_set;                                       // _RST_DEV_X_SET_0,          0x290
  UINT32 crc_rst_dev_x_clr;                                       // _RST_DEV_X_CLR_0,          0x294
  UINT32 crc_clk_out_enb_y;                                       // _CLK_OUT_ENB_Y_0,          0x298
  UINT32 crc_clk_enb_y_set;                                       // _CLK_ENB_Y_SET_0,          0x29c
  UINT32 crc_clk_enb_y_clr;                                       // _CLK_ENB_Y_CLR_0,          0x2a0
  UINT32 crc_rst_devices_y;                                       // _RST_DEVICES_Y_0,          0x2a4
  UINT32 crc_rst_dev_y_set;                                       // _RST_DEV_Y_SET_0,          0x2a8
  UINT32 crc_rst_dev_y_clr;                                       // _RST_DEV_Y_CLR_0,          0x2ac
  UINT32 crc_reserved21[17];                                      // _reserved_21,              0x2b0 - 0x2f0
  UINT32 crc_dfll_base;                                           // _DFLL_BASE_0,              0x2f4
  UINT32 crc_reserved22[2];                                       // _reserved_22,              0x2f8 - 0x2fc

  ClockSetClr crc_rst_dev_ex[TEGRA_CLK_REGS];                     // _RST_DEV_L/H/U_SET_0,      0x300 ~ 0x314

  UINT32 crc_reserved30[2];                                       // _reserved_30,              0x318,  0x31c

  ClockSetClr crc_clk_enb_ex[TEGRA_CLK_REGS];                     // _CLK_ENB_L/H/U_CLR_0,      0x320 ~ 0x334

  UINT32 crc_reserved31[2];                                       // _reserved_31,              0x338,  0x33c
  UINT32 crc_cpu_cmplx_set;                                       // _RST_CPU_CMPLX_SET_0,      0x340
  UINT32 crc_cpu_cmplx_clr;                                       // _RST_CPU_CMPLX_CLR_0,      0x344

  // Additional Tegra 30 Registers
  UINT32 crc_clk_cpu_cmplx_set;                                   // _CLK_CPU_CMPLX_SET_0,      0x348
  UINT32 crc_clk_cpu_cmplx_clr;                                   // _CLK_CPU_CMPLX_SET_0,      0x34c
  UINT32 crc_reserved32[2];                                       // _reserved_32,              0x350,  0x354
  UINT32 crc_rst_dev_vw[TEGRA_CLK_REGS_VW];                       // _RST_DEVICES_V/W_0
  UINT32 crc_clk_out_enb_vw[TEGRA_CLK_REGS_VW];                   // _CLK_OUT_ENB_V/W_0
  UINT32 crc_cclkg_brst_pol;                                      // _CCLKG_BURST_POLICY_0,     0x368
  UINT32 crc_super_cclkg_div;                                     // _SUPER_CCLKG_DIVIDER_0,    0x36C
  UINT32 crc_cclklp_brst_pol;                                     // _CCLKLP_BURST_POLICY_0,    0x370
  UINT32 crc_super_cclkp_div;                                     // _SUPER_CCLKLP_DIVIDER_0,   0x374
  UINT32 crc_clk_cpug_cmplx;                                      // _CLK_CPUG_CMPLX_0,         0x378
  UINT32 crc_clk_cpulp_cmplx;                                     // _CLK_CPULP_CMPLX_0,        0x37C
  UINT32 crc_cpu_softrst_ctrl;                                    // _CPU_SOFTRST_CTRL_0,       0x380
  UINT32 crc_cpu_softrst_ctrl1;                                   // _CPU_SOFTRST_CTRL1_0,      0x384
  UINT32 crc_cpu_softrst_ctrl2;                                   // _CPU_SOFTRST_CTRL2_0,      0x388
  UINT32 crc_reserved33[9];                                       // _reserved_33,              0x38c - 0x3ac
  UINT32 crc_clk_src_vw[TEGRA_CLK_SOURCES_VW];                    //                            0x3B0 - 0x42C

  ClockSetClr crc_rst_dev_ex_vw[TEGRA_CLK_REGS_VW];               // _RST_DEV_V/W_SET_0,        0x430 ~ 0x43c
  ClockSetClr crc_clk_enb_ex_vw[TEGRA_CLK_REGS_VW];               // _CLK_ENB_V/W_CLR_0,        0x440 ~ 0x44c

  // Additional Tegra 114+ Registers
  UINT32 crc_rst_cpug_cmplx_set;                                  // _RST_CPUG_CMPLX_SET_0,     0x450
  UINT32 crc_rst_cpug_cmplx_clr;                                  // _RST_CPUG_CMPLX_CLR_0,     0x454
  UINT32 crc_rst_cpulp_cmplx_set;                                 // _RST_CPULP_CMPLX_SET_0,    0x458
  UINT32 crc_rst_cpulp_cmplx_clr;                                 // _RST_CPULP_CMPLX_CLR_0,    0x45C
  UINT32 crc_clk_cpug_cmplx_set;                                  // _CLK_CPUG_CMPLX_SET_0,     0x460
  UINT32 crc_clk_cpug_cmplx_clr;                                  // _CLK_CPUG_CMPLX_CLR_0,     0x464
  UINT32 crc_clk_cpulp_cmplx_set;                                 // _CLK_CPULP_CMPLX_SET_0,    0x468
  UINT32 crc_clk_cpulp_cmplx_clr;                                 // _CLK_CPULP_CMPLX_CLR_0,    0x46C
  UINT32 crc_cpu_cmplx_status;                                    // _CPU_CMPLX_STATUS_0,       0x470
  UINT32 crc_reserved40[1];                                       // _reserved_40,              0x474
  UINT32 crc_intstatus;                                           // __INTSTATUS_0,             0x478
  UINT32 crc_intmask;                                             // __INTMASK_0,               0x47C
  UINT32 crc_utmip_pll_cfg0;                                      // _UTMIP_PLL_CFG0_0,         0x480
  UINT32 crc_utmip_pll_cfg1;                                      // _UTMIP_PLL_CFG1_0,         0x484
  UINT32 crc_utmip_pll_cfg2;                                      // _UTMIP_PLL_CFG2_0,         0x488
  UINT32 crc_plle_aux;                                            // _PLLE_AUX_0,               0x48C
  UINT32 crc_sata_pll_cfg0;                                       // _SATA_PLL_CFG0_0,          0x490
  UINT32 crc_sata_pll_cfg1;                                       // _SATA_PLL_CFG1_0,          0x494
  UINT32 crc_pcie_pll_cfg0;                                       // _PCIE_PLL_CFG0_0,          0x498
  UINT32 crc_prog_audio_dly_clk;                                  // _PROG_AUDIO_DLY_CLK_0,     0x49C
  UINT32 crc_audio_sync_clk_i2s0;                                 // _AUDIO_SYNC_CLK_I2S0_0,    0x4A0
  UINT32 crc_audio_sync_clk_i2s1;                                 // _AUDIO_SYNC_CLK_I2S1_0,    0x4A4
  UINT32 crc_audio_sync_clk_i2s2;                                 // _AUDIO_SYNC_CLK_I2S2_0,    0x4A8
  UINT32 crc_audio_sync_clk_i2s3;                                 // _AUDIO_SYNC_CLK_I2S3_0,    0x4AC
  UINT32 crc_audio_sync_clk_i2s4;                                 // _AUDIO_SYNC_CLK_I2S4_0,    0x4B0
  UINT32 crc_audio_sync_clk_spdif;                                // _AUDIO_SYNC_CLK_SPDIF_0,   0x4B4

  SimpleClockPll plld2;                                           // _PLLD2_BASE_0,             0x4B8

  UINT32 crc_utmip_pll_cfg3;                                      // _UTMIP_PLL_CFG3_0,         0x4C0
  UINT32 crc_pllrefe_base;                                        // _PLLREFE_BASE_0,           0x4C4
  UINT32 crc_pllrefe_misc;                                        // _PLLREFE_MISC_0,           0x4C8
  UINT32 crs_reserved_50[7];                                      // _reserved_50,              0x4CC - 0x4E4
  UINT32 crc_pllc2_base;                                          // _PLLC2_BASE_0,             0x4E8
  UINT32 crc_pllc2_misc0;                                         // _PLLC2_MISC_0_0,           0x4EC
  UINT32 crc_pllc2_misc1;                                         // _PLLC2_MISC_1_0,           0x4F0
  UINT32 crc_pllc2_misc2;                                         // _PLLC2_MISC_2_0,           0x4F4
  UINT32 crc_pllc2_misc3;                                         // _PLLC2_MISC_3_0,           0x4F8
  UINT32 crc_pllc3_base;                                          // _PLLC3_BASE_0,             0x4FC
  UINT32 crc_pllc3_misc0;                                         // _PLLC3_MISC_0_0,           0x500
  UINT32 crc_pllc3_misc1;                                         // _PLLC3_MISC_1_0,           0x504
  UINT32 crc_pllc3_misc2;                                         // _PLLC3_MISC_2_0,           0x508
  UINT32 crc_pllc3_misc3;                                         // _PLLC3_MISC_3_0,           0x50C
  UINT32 crc_pllx_misc1;                                          // _PLLX_MISC_1_0,            0x510
  UINT32 crc_pllx_misc2;                                          // _PLLX_MISC_2_0,            0x514
  UINT32 crc_pllx_misc3;                                          // _PLLX_MISC_3_0,            0x518
  UINT32 crc_xusbio_pll_cfg0;                                     // _XUSBIO_PLL_CFG0_0,        0x51C
  UINT32 crc_xusbio_pll_cfg1;                                     // _XUSBIO_PLL_CFG0_1,        0x520
  UINT32 crc_plle_aux1;                                           // _PLLE_AUX1_0,              0x524
  UINT32 crc_pllp_reshift;                                        // _PLLP_RESHIFT_0,           0x528
  UINT32 crc_utmipll_hw_pwrdn_cfg0;                               // _UTMIPLL_HW_PWRDN_CFG0_0,  0x52C
  UINT32 crc_pllu_hw_pwrdn_cfg0;                                  // _PLLU_HW_PWRDN_CFG0_0,     0x530
  UINT32 crc_xusb_pll_cfg0;                                       // _XUSB_PLL_CFG0_0,          0x534
  UINT32 crc_reserved51[1];                                       // _reserved_51,              0x538
  UINT32 crc_clk_cpu_misc;                                        // _CLK_CPU_MISC_0,           0x53C
  UINT32 crc_clk_cpug_misc;                                       // _CLK_CPUG_MISC_0,          0x540
  UINT32 crc_clk_cpulp_misc;                                      // _CLK_CPULP_MISC_0,         0x544
  UINT32 crc_pllx_hw_ctrl_cfg;                                    // _PLLX_HW_CTRL_CFG_0,       0x548
  UINT32 crc_pllx_sw_ramp_cfg;                                    // _PLLX_SW_RAMP_CFG_0,       0x54C
  UINT32 crc_pllx_hw_ctrl_status;                                 // _PLLX_HW_CTRL_STATUS_0,    0x550
  UINT32 crc_reserved52[1];                                       // _reserved_52,              0x554
  UINT32 crc_super_gr3d_clk_div;                                  // _SUPER_GR3D_CLK_DIVIDER_0, 0x558
  UINT32 crc_spare_reg0;                                          // _SPARE_REG0_0,             0x55C
  UINT32 _rsv32[4];                                               //                            0x560 - 0x56c
  UINT32 crc_plld2_ss_cfg;                                        // _PLLD2_SS_CFG,             0x570
  UINT32 _rsv32_1[7];                                             //                            0x574 - 0x58c

  SimpleClockPll plldp;                                           // _PLLDP_BASE / _PLLDP_MISC, 0x590

  UINT32 crc_plldp_ss_cfg;                                        // _PLLDP_SS_CFG,             0x598

  // Tegra124+ - Skip to 0x600 here for new CLK_SOURCE_ Registers
  UINT32 _rsrv32_2[25];                                           //                            0x59C - 0x5FC
  UINT32 crc_clk_src_x[TEGRA_CLK_SOURCES_X];                      // XUSB, etc,                 0x600 - 0x67C

  // Tegra210 - Skip to 0x694 here for new CLK_SOURCE_ Registers
  UINT32 crc_reserved61[5];                                       // _reserved_61,              0x680 - 0x690

  //
  // NOTE: PLLA1 Registers are in the middle of this Y Region. Break this in
  // two later if PLLA1 is needed, but for now this is cleaner.
  //
  UINT32 crc_clk_src_y[TEGRA_CLK_SOURCES_Y];                       // SPARE1, etc,              0x694 - 0x6D8
} ClockResetController;

// Set of Oscillator Frequencies Supported in the Internal API.
enum OscClockFreq {
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

#endif /* _CLOCK_H_ */
