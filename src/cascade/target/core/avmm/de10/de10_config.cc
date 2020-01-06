// Copyright 2017-2019 VMware, Inc.
// SPDX-License-Identifier: BSD-2-Clause
//
// The BSD-2 license (the License) set forth below applies to all parts of the
// Cascade project.  You may not use this file except in compliance with the
// License.
//
// BSD-2 License
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are met:
//
// 1. Redistributions of source code must retain the above copyright notice, this
// list of conditions and the following disclaimer.
//
// 2. Redistributions in binary form must reproduce the above copyright notice,
// this list of conditions and the following disclaimer in the documentation
// and/or other materials provided with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS AS IS AND
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// MIT License
//
// Copyright (c) 2018 Nicolás Hasbún
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "target/core/avmm/de10/de10_config.h"

#include <cstdio>    
#include <cstdlib>   
#include <cstring>   
#include <stdint.h>   
#include <fcntl.h>    
#include <sys/mman.h> 
#include <unistd.h>
#include "target/core/avmm/de10/hps.h"
#include "target/core/avmm/de10/socal.h"

// Useful macros
#define BIT(x,n) (((x) >> (n)) & 1)
#define INSERT_BITS(original, mask, value, num) (original & (~mask)) | (value << num)

#define FPGA_MANAGER_ADD (0xff706000) // FPGA MANAGER MAIN REGISTER ADDRESS
#define STAT_OFFSET      (0x000)
#define CTRL_OFFSET      (0x004)
#define GPIO_INTSTATUS   (0x840)

#define FPGA_MANAGER_DATA_ADD (0xffb90000) // FPGA MANAGER DATA REGISTER ADDRESS

using namespace std;

namespace cascade::avmm {

void De10Config::run(const string& path) {
  fd_ = open("/dev/mem", (O_RDWR|O_SYNC));

  const size_t largobytes = 1;
  virtualbase_ = mmap(NULL, largobytes,
   (PROT_READ|PROT_WRITE), MAP_SHARED, fd_, FPGA_MANAGER_ADD);
  config_routine(path.c_str());

  close(fd_);
}

void De10Config::config_routine(const char* path) {
  report_status(); // Check registers... could be accessed using "status" argument.
  set_ctrl_en(1);  // Activate control by HPS.
  fpga_off();      // Reset State for fpga.
  while(1) if(fpga_state() == 0x1) break;

  set_cdratio();   // Set corresponding cdratio (check fpga manager docs).
  fpga_on();       // Should be on config state.
  while(1) if(fpga_state() == 0x2) break;

  set_axicfgen(1); // Activate AXI configuration data transfers.
  config_fpga(path);   // Load rbf config file to fpga manager data register.
  while(1) if(fpga_state() == 0x4) break;

  set_axicfgen(0); // Turn off AXI configuration data transfers..
  set_ctrl_en(0);  // Disable control by HPS (so JTAG can load new fpga configs).
  report_status(); // Should report "User mode".
}

void De10Config::report_status() {
  uint8_t status = alt_read_byte(static_cast<uint8_t*>(virtualbase_) + STAT_OFFSET);
  uint8_t mode_mask = 0b111;
  uint8_t msel_mask = 0b11111 << 3;

  uint8_t mode = status & mode_mask;
  uint8_t msel = (status & msel_mask) >> 3;

  uint16_t control_reg = alt_read_hword(static_cast<uint8_t*>(virtualbase_) + CTRL_OFFSET);
  uint16_t cfgwdth_mask = (0b1 << 9);
  uint16_t cdratio_mask = (0b11 << 6);

  uint8_t cfgwdth  = (control_reg & cfgwdth_mask) >> 9;
  uint8_t cdratio  = (control_reg & cdratio_mask) >> 6;
  uint8_t axicfgen = BIT(control_reg, 8);
  uint8_t ctrl_en  = BIT(control_reg, 0);
  uint8_t nconfigpull = BIT(control_reg, 2);

  uint16_t gpio_intstatus_reg  = alt_read_hword(static_cast<uint8_t*>(virtualbase_) + GPIO_INTSTATUS);
  uint8_t cd  = BIT(gpio_intstatus_reg, 1); // configuration done register
}

uint8_t De10Config::fpga_state() {
  uint8_t status = alt_read_byte(static_cast<uint8_t*>(virtualbase_) + STAT_OFFSET);
  uint8_t mode_mask = 0b111;
  uint8_t mode = status & mode_mask;
  return mode;
}

void De10Config::set_cdratio() {
  uint16_t control_reg  = alt_read_hword(static_cast<uint8_t*>(virtualbase_) + CTRL_OFFSET);
  uint16_t cdratio_mask = (0b11 << 6);
  uint8_t  cdratio      = 0x3;

  control_reg = INSERT_BITS(control_reg, cdratio_mask, cdratio, 6);
  alt_write_hword(static_cast<uint8_t*>(virtualbase_) + CTRL_OFFSET, control_reg);
}

void De10Config::config_fpga(const char* path) {
  // Memory map the fpga data register
  void * data_mmap = mmap(NULL, 4,
   (PROT_READ|PROT_WRITE), MAP_SHARED, fd_, FPGA_MANAGER_DATA_ADD);

  // Open rbf file.
  int rbf = open(path, (O_RDONLY|O_SYNC));
  if (rbf < 0) {
    // Some error happened...
    //printf("\n%s\n\n", Error opening file. Check for an appropiate fpga_config_file.rbf file.");
    exit(-1);
  }

  // Set buffer to read rbf files and copy to fpga manager data register.
  uint8_t * data_buffer = (uint8_t*)malloc(sizeof(uint8_t) * 4);
  memset(data_buffer, 0, 4); // set initial data to 0.

  // Loop to read rbf and write to fpga data address.
  // We advancse every 4 bytes (32 bits).

  bool run_while = true;
  //printf("%s\n", "Loading rbf file.");

  while(run_while) {
    ssize_t read_result = read(rbf, data_buffer, 4);
    if (read_result < 4) {
      //printf("%s\n", "EOF reached.");
      run_while = false;
    }

    // Follow format expected by fpga manager.
    uint32_t format_data = *(data_buffer)          << 0;
    format_data = format_data | *(data_buffer + 1) << 8;
    format_data = format_data | *(data_buffer + 2) << 16;
    format_data = format_data | *(data_buffer + 3) << 24;

    alt_write_word(data_mmap, format_data);
    memset(data_buffer, 0, 4); // reset data to 0.
  }

  close(rbf);
}

void De10Config::set_axicfgen(uint8_t value) {
  uint16_t control_reg  = alt_read_hword(static_cast<uint8_t*>(virtualbase_) + CTRL_OFFSET);
  uint16_t axicfgen_mask = 1 << 8;
  uint8_t axicfgen = value & 1; // binary values

  control_reg = INSERT_BITS(control_reg, axicfgen_mask, axicfgen, 8);

  alt_write_hword(static_cast<uint8_t*>(virtualbase_) + CTRL_OFFSET, control_reg);
}

void De10Config::set_ctrl_en(uint8_t value) {
  uint16_t control_reg  = alt_read_hword(static_cast<uint8_t*>(virtualbase_) + CTRL_OFFSET);
  uint16_t ctrl_en_mask = 1 << 0;
  uint8_t  ctrl_en = value & 1; // binary values

  control_reg = INSERT_BITS(control_reg, ctrl_en_mask, ctrl_en, 0);

  alt_write_hword(static_cast<uint8_t*>(virtualbase_) + CTRL_OFFSET, control_reg);
}

void De10Config::set_nconfigpull(uint8_t value) {
  uint16_t control_reg  = alt_read_hword(static_cast<uint8_t*>(virtualbase_) + CTRL_OFFSET);
  uint16_t nconfigpull_mask = 1 << 2;
  uint8_t  nconfigpull = value & 1; // binary values

  control_reg = INSERT_BITS(control_reg, nconfigpull_mask, nconfigpull, 2);

  alt_write_hword(static_cast<uint8_t*>(virtualbase_) + CTRL_OFFSET, control_reg);
}

void De10Config::fpga_off() {
  set_nconfigpull(1);
}

void De10Config::fpga_on() {
  set_nconfigpull(0);
}

} // namespace cascade::avmm
