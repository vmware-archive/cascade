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

#include <stdio.h>    // printf
#include <stdint.h>   // uint8_t
#include <stdlib.h>   // malloc
#include <stdbool.h>  // true false
#include <unistd.h>   // sleep
#include <string.h>   // string
#include <fcntl.h>    // open
#include <sys/mman.h> // mmap

// The Altera SoC Abstraction Layer (SoCAL) API Reference Manual
#include "de10/include/socal.h"
#include "de10/include/hps.h"

// Useful macros
#define BIT(x,n) (((x) >> (n)) & 1)
#define INSERT_BITS(original, mask, value, num) (original & (~mask)) | (value << num)

// Main function declarations
void config_routine(const char* path);

void report_status();
uint8_t fpga_state();
void set_cdratio();
void reset_fpga();
void config_fpga(const char* path);
void set_axicfgen(uint8_t value);
void set_ctrl_en(uint8_t value);
void set_nconfigpull(uint8_t value);
void fpga_off();
void fpga_on();

// Auxiliary functions
char * status_code(uint8_t code);

#define FPGA_MANAGER_ADD (0xff706000) // FPGA MANAGER MAIN REGISTER ADDRESS
#define STAT_OFFSET      (0x000)
#define CTRL_OFFSET      (0x004)
#define GPIO_INTSTATUS   (0x840)

#define FPGA_MANAGER_DATA_ADD (0xffb90000) // FPGA MANAGER DATA REGISTER ADDRESS

int fd; // file descriptor for memory access
void * virtualbase; // puntero genérico con map de userspace a hw

int main(int argc, const char * argv[])
{
  const size_t largobytes = 1;

  fd = open("/dev/mem", (O_RDWR|O_SYNC));

  virtualbase = mmap(NULL, largobytes,
   (PROT_READ|PROT_WRITE), MAP_SHARED, fd, FPGA_MANAGER_ADD);
  // MAP_SHARED comparte la memoria con otras apicaciones
  // PROT READ y PROT WRITE, para lectura y escritura

  if(argc > 1) {
    if(strcmp(argv[1], "report_status") == 0)        report_status();
    else if(strcmp(argv[1], "status") == 0)          report_status();
    else if(strcmp(argv[1], "set_cdratio") == 0)     set_cdratio();
    else if(strcmp(argv[1], "reset_fpga") == 0)      reset_fpga();
    else if(strcmp(argv[1], "reset") == 0)           reset_fpga();
    else if(strcmp(argv[1], "config_fpga") == 0)     config_fpga(argv[2]);
    else if(strcmp(argv[1], "set_axicfgen") == 0)    set_axicfgen(atoi(argv[2]));
    else if(strcmp(argv[1], "fpga_off") == 0)        fpga_off();
    else if(strcmp(argv[1], "fpga_on") == 0)         fpga_on();
    else if(strcmp(argv[1], "set_ctrl_en") == 0)     set_ctrl_en(atoi(argv[2]));
    else if(strcmp(argv[1], "set_nconfigpull") == 0) set_nconfigpull(atoi(argv[2]));
    else if(strcmp(argv[1], "program") == 0)         config_routine(argv[2]);
  }

  close(fd);
  return 0;
}

void report_status()
// Status reg report MSEL (RO) config and FPGA current state (RW).
// Also report cfgwdth, cdratio registers and other useful registers.
{
  uint8_t status = alt_read_byte(virtualbase + STAT_OFFSET);
  uint8_t mode_mask = 0b111;
  uint8_t msel_mask = 0b11111 << 3;

  uint8_t mode = status & mode_mask;
  uint8_t msel = (status & msel_mask) >> 3;

  uint16_t control_reg = alt_read_hword(virtualbase + CTRL_OFFSET);
  uint16_t cfgwdth_mask = (0b1 << 9);
  uint16_t cdratio_mask = (0b11 << 6);

  uint8_t cfgwdth  = (control_reg & cfgwdth_mask) >> 9;
  uint8_t cdratio  = (control_reg & cdratio_mask) >> 6;
  uint8_t axicfgen = BIT(control_reg, 8);
  uint8_t ctrl_en  = BIT(control_reg, 0);
  uint8_t nconfigpull = BIT(control_reg, 2);


  uint16_t gpio_intstatus_reg  = alt_read_hword(virtualbase + GPIO_INTSTATUS);
  uint8_t cd  = BIT(gpio_intstatus_reg, 1); // configuration done register

  /*
  printf("%s\n",      "******************************************************");
  printf("%s 0x%x\n", "MSEL Pin Config.....", msel);
  printf("%s %s\n",   "FPGA State..........", status_code(mode) );
  printf("%s 0x%x\n", "cfgwdth Register....", cfgwdth);
  printf("%s 0x%x\n", "cdratio Register....", cdratio);
  printf("%s 0x%x\n", "axicfgen Register...", axicfgen);
  printf("%s 0x%x\n", "Nconfig pull reg....", nconfigpull);
  printf("%s 0x%x\n", "CONF DONE...........", cd);
  printf("%s 0x%x\n", "Ctrl.en?............", ctrl_en);
  printf("%s\n",      "******************************************************");
  */
}

uint8_t fpga_state()
{
  uint8_t status = alt_read_byte(virtualbase + STAT_OFFSET);
  uint8_t mode_mask = 0b111;
  uint8_t mode = status & mode_mask;

  return mode;
}

void set_cdratio()
// This should match your MSEL Pin configuration.
// This is a config for MSEL[4..0] = 01010
{
  uint16_t control_reg  = alt_read_hword(virtualbase + CTRL_OFFSET);
  uint16_t cdratio_mask = (0b11 << 6);
  uint8_t  cdratio      = 0x3;

  control_reg = INSERT_BITS(control_reg, cdratio_mask, cdratio, 6);
  alt_write_hword(virtualbase + CTRL_OFFSET, control_reg);

  //printf("%s 0x%x.\n", "Setting cdratio with", cdratio);
}

void reset_fpga()
{
  uint8_t status = alt_read_byte(virtualbase + STAT_OFFSET);
  uint8_t mode_mask = 0b111;

  status = INSERT_BITS(status, mode_mask, 0x1, 0);

  alt_write_byte(virtualbase + STAT_OFFSET, status);

  //printf("%s.\n", "Resetting FPGA");
}

void config_fpga(const char* path)
// Main routine to transfer rbf file to fpga manager data register.
{
  // Memory map the fpga data register
  void * data_mmap = mmap(NULL, 4,
   (PROT_READ|PROT_WRITE), MAP_SHARED, fd, FPGA_MANAGER_DATA_ADD);

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

void set_axicfgen(uint8_t value)
{
  uint16_t control_reg  = alt_read_hword(virtualbase + CTRL_OFFSET);
  uint16_t axicfgen_mask = 1 << 8;
  uint8_t axicfgen = value & 1; // binary values

  control_reg = INSERT_BITS(control_reg, axicfgen_mask, axicfgen, 8);

  alt_write_hword(virtualbase + CTRL_OFFSET, control_reg);
}

void set_ctrl_en(uint8_t value)
{
  uint16_t control_reg  = alt_read_hword(virtualbase + CTRL_OFFSET);
  uint16_t ctrl_en_mask = 1 << 0;
  uint8_t  ctrl_en = value & 1; // binary values

  control_reg = INSERT_BITS(control_reg, ctrl_en_mask, ctrl_en, 0);

  alt_write_hword(virtualbase + CTRL_OFFSET, control_reg);
}

void set_nconfigpull(uint8_t value)
{
  uint16_t control_reg  = alt_read_hword(virtualbase + CTRL_OFFSET);
  uint16_t nconfigpull_mask = 1 << 2;
  uint8_t  nconfigpull = value & 1; // binary values

  control_reg = INSERT_BITS(control_reg, nconfigpull_mask, nconfigpull, 2);

  alt_write_hword(virtualbase + CTRL_OFFSET, control_reg);
}

void fpga_off()
{
  set_nconfigpull(1);
  //printf("%s.\n", "Turning FPGA Off");
}

void fpga_on()
{
  set_nconfigpull(0);
  //printf("%s.\n", "Turning FPGA On");
}

// ****************************************************************************
// *                            Auxiliary functions                           *
// ****************************************************************************

char * status_code(uint8_t code)
{
  char * description = (char *)malloc(sizeof(char) * 30);

  if (code == 0x0)      description = "Powered Off";
  else if (code == 0x1) description = "Reset Phase";
  else if (code == 0x2) description = "Configuration Phase";
  else if (code == 0x3) description = "Initialization Phase";
  else if (code == 0x4) description = "User Phase";
  else                  description = "Undetermined (error ?)";

  return description;
}

// ****************************************************************************
// *                          Complete config routine                         *
// ****************************************************************************

void config_routine(const char* path)
{
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
