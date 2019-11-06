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

#ifndef	_PRELOADER_BUILD_H_
#define	_PRELOADER_BUILD_H_

/*
 * Boot option. 1 mean that particular boot mode is selected.
 * Only 1 boot option to be enabled at any time
 */
#define CONFIG_PRELOADER_BOOT_FROM_QSPI		(0)
#define CONFIG_PRELOADER_BOOT_FROM_SDMMC	(1)
#define CONFIG_PRELOADER_BOOT_FROM_NAND		(0)
#define CONFIG_PRELOADER_BOOT_FROM_RAM		(0)

/*
 * Handoff files must provide image location of subsequent
 * bootloader inside the boot devices / flashes
 */
#if (CONFIG_PRELOADER_BOOT_FROM_QSPI == 1)
#define CONFIG_PRELOADER_QSPI_NEXT_BOOT_IMAGE	(0x60000)
#endif
#if (CONFIG_PRELOADER_BOOT_FROM_SDMMC == 1)
#define CONFIG_PRELOADER_SDMMC_NEXT_BOOT_IMAGE	(0x40000)
#endif
#if (CONFIG_PRELOADER_BOOT_FROM_NAND == 1)
#define CONFIG_PRELOADER_NAND_NEXT_BOOT_IMAGE	(0xc0000)
#endif

/* Enable FAT partition support when booting from SDMMC. */
#define CONFIG_PRELOADER_FAT_SUPPORT		(0)

/*
 * When FAT partition support is enabled, this specifies the
 * FAT partition where the boot image is located.
 */
#define CONFIG_PRELOADER_FAT_BOOT_PARTITION	(1)

/*
 * When FAT partition supported is enabled, this specifies the
 * boot image filename within a FAT partition to be used as
 * fatload payload.
 */
#define CONFIG_PRELOADER_FAT_LOAD_PAYLOAD_NAME 	"u-boot.img"

/*
 * Handoff files must provide user option whether to
 * enable watchdog during preloader execution phase
 */
#define CONFIG_PRELOADER_WATCHDOG_ENABLE	(1)

/*
 * Handoff files must provide user option whether to enable
 * debug memory write support
 */
#define CONFIG_PRELOADER_DEBUG_MEMORY_WRITE	(0)
/* the base address of debug memory */
#if (CONFIG_PRELOADER_DEBUG_MEMORY_WRITE == 1)
#define CONFIG_PRELOADER_DEBUG_MEMORY_ADDR	(0xfffffd00)
#define CONFIG_PRELOADER_DEBUG_MEMORY_SIZE	(0x200)
#endif

/* Semihosting support in Preloader */
#define CONFIG_PRELOADER_SEMIHOSTING		(0)

/* Option to check checksum of subsequent boot software image */
#define CONFIG_PRELOADER_CHECKSUM_NEXT_IMAGE	(1)

/*
 * Handoff files must provide user option whether to enable
 * debug serial printout support
 */
#define CONFIG_PRELOADER_SERIAL_SUPPORT		(1)

/*
 * Handoff files must provide user option whether to enable
 * hardware diagnostic support
 */
#define CONFIG_PRELOADER_HARDWARE_DIAGNOSTIC	(0)

/*
 * Preloader execute on FPGA. This is normally selected
 * for BootROM FPGA boot where Preloader located on FPGA
 */
#define CONFIG_PRELOADER_EXE_ON_FPGA		(0)
#if (CONFIG_PRELOADER_EXE_ON_FPGA == 1)
#define CONFIG_FPGA_MAX_SIZE			(0x10000)
#define CONFIG_FPGA_DATA_BASE			0xffff0000
#define CONFIG_FPGA_DATA_MAX_SIZE		(0x10000)
#endif

/*
 * Add new option to force ramboot pll reset
 */
#define CONFIG_PRELOADER_RAMBOOT_PLLRESET	(1)

/*
 * Enabled write STATE_VALID value to STATE_REG register to
 * tell BootROM that Preloader run successfully.
 */
#define CONFIG_PRELOADER_STATE_REG_ENABLE	(1)

/*
 * Enabled the handshake with BootROM when confiuring the IOCSR and pin mux.
 * If enabled and warm reset happen in middle of Preloader configuring IOCSR
 * and pin mux, BootROM will reconfigure the IOCSR and pin mux again.
 */
#define CONFIG_PRELOADER_BOOTROM_HANDSHAKE_CFGIO	(1)

/*
 * If enabled, when warm reset happen and BootROM skipped configuring IOCSR
 * and pin mux, Preloader will skip configuring the IOCSR and pin mux too.
 */
#define CONFIG_PRELOADER_WARMRST_SKIP_CFGIO	(1)

/*
 * If enabled, Preloader will skip SDRAM initialization and calibration.
 */
#define CONFIG_PRELOADER_SKIP_SDRAM		(0)

/*
 * To configure whether to scrub the SDRAM to initialize the ECC bits
 */
#define CONFIG_PRELOADER_SDRAM_SCRUBBING    (0)

/* To configure whether to scrub the SDRAM to initialize the ECC bits */
#if (CONFIG_PRELOADER_SDRAM_SCRUBBING == 1)
/*
 * The region of next stage boot image will be copied to
 */
#define CONFIG_PRELOADER_SDRAM_SCRUB_BOOT_REGION_START    (0x1000000)
#define CONFIG_PRELOADER_SDRAM_SCRUB_BOOT_REGION_END    (0x2000000)
/*
 * Decide remaining region will be scrubbed. This will be done during the flash
 * access (to load next boot image). The region is auto calculated based on the
 * remain region. For SOCFPAGA, it would be 2 regions as below
 * > CONFIG_SYS_SDRAM_BASE to CONFIG_SPL_SDRAM_SCRUB_BOOT_REGION_START
 * > CONFIG_SPL_SDRAM_SCRUB_BOOT_REGION_END to calculated SDRAM size
 */
#define CONFIG_PRELOADER_SDRAM_SCRUB_REMAIN_REGION    (1)

#endif /* CONFIG_PRELOADER_SDRAM_SCRUBBING */

#endif /* _PRELOADER_BUILD_H_ */


