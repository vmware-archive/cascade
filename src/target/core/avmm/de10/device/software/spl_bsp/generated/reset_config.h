/* GENERATED FILE - DO NOT EDIT */
/*
 * Copyright Altera Corporation (C) 2012-2014. All rights reserved
 *
 * SPDX-License-Identifier:    BSD-3-Clause
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *    * Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *    * Redistributions in binary form must reproduce the above copyright
 *      notice, this list of conditions and the following disclaimer in the
 *      documentation and/or other materials provided with the distribution.
 *    * Neither the name of Altera Corporation nor the
 *      names of its contributors may be used to endorse or promote products
 *      derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL ALTERA CORPORATION BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _PRELOADER_RESET_CONFIG_H_
#define _PRELOADER_RESET_CONFIG_H_

/* 1 mean that particular IP need to be kept under reset state */
#define CONFIG_HPS_RESET_ASSERT_EMAC0		(1)
#define CONFIG_HPS_RESET_ASSERT_EMAC1		(0)
#define CONFIG_HPS_RESET_ASSERT_USB0		(1)
#define CONFIG_HPS_RESET_ASSERT_USB1		(0)
#define CONFIG_HPS_RESET_ASSERT_NAND		(1)
#define CONFIG_HPS_RESET_ASSERT_SDMMC		(0)
#define CONFIG_HPS_RESET_ASSERT_QSPI		(1)
#define CONFIG_HPS_RESET_ASSERT_UART0		(0)
#define CONFIG_HPS_RESET_ASSERT_UART1		(1)
#define CONFIG_HPS_RESET_ASSERT_I2C0		(0)
#define CONFIG_HPS_RESET_ASSERT_I2C1		(0)
#define CONFIG_HPS_RESET_ASSERT_I2C2		(1)
#define CONFIG_HPS_RESET_ASSERT_I2C3		(1)
#define CONFIG_HPS_RESET_ASSERT_SPIM0		(1)
#define CONFIG_HPS_RESET_ASSERT_SPIM1		(0)
#define CONFIG_HPS_RESET_ASSERT_SPIS0		(1)
#define CONFIG_HPS_RESET_ASSERT_SPIS1		(1)
#define CONFIG_HPS_RESET_ASSERT_CAN0		(1)
#define CONFIG_HPS_RESET_ASSERT_CAN1		(1)
#define CONFIG_HPS_RESET_ASSERT_L4WD1		(0)
#define CONFIG_HPS_RESET_ASSERT_OSC1TIMER1	(0)
#define CONFIG_HPS_RESET_ASSERT_SPTIMER0	(0)
#define CONFIG_HPS_RESET_ASSERT_SPTIMER1	(0)
#define CONFIG_HPS_RESET_ASSERT_GPIO0		(0)
#define CONFIG_HPS_RESET_ASSERT_GPIO1		(0)
#define CONFIG_HPS_RESET_ASSERT_GPIO2		(0)
#define CONFIG_HPS_RESET_ASSERT_DMA		(0)
#define CONFIG_HPS_RESET_ASSERT_SDR		(0)

#define CONFIG_HPS_RESET_ASSERT_FPGA_DMA0	(1)
#define CONFIG_HPS_RESET_ASSERT_FPGA_DMA1	(1)
#define CONFIG_HPS_RESET_ASSERT_FPGA_DMA2	(1)
#define CONFIG_HPS_RESET_ASSERT_FPGA_DMA3	(1)
#define CONFIG_HPS_RESET_ASSERT_FPGA_DMA4	(1)
#define CONFIG_HPS_RESET_ASSERT_FPGA_DMA5	(1)
#define CONFIG_HPS_RESET_ASSERT_FPGA_DMA6	(1)
#define CONFIG_HPS_RESET_ASSERT_FPGA_DMA7	(1)

#define CONFIG_HPS_RESET_ASSERT_HPS2FPGA	(0)
#define CONFIG_HPS_RESET_ASSERT_LWHPS2FPGA	(0)
#define CONFIG_HPS_RESET_ASSERT_FPGA2HPS	(0)

#define CONFIG_HPS_RESET_WARMRST_HANDSHAKE_FPGA		(1)
#define CONFIG_HPS_RESET_WARMRST_HANDSHAKE_ETR		(1)
#define CONFIG_HPS_RESET_WARMRST_HANDSHAKE_SDRAM	(0)

#endif /* _PRELOADER_RESET_CONFIG_H_ */


