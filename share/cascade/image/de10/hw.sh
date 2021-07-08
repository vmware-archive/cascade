#!/bin/sh

# This script compiles the hardware side of the de10 image:
#   1. The default bitstream data/soc_system.rbf
#   2. The preloader data/preloader-mkpimage.bin
#   3. The device tree data/soc_system.dtb

# Because this script is both memory and time-consuming (the quartus
# SDE requires several gigabytes of installation space) and the results
# are modestly sized O(2MB), we've added these files to the repository.
# In general, it should not be necessary to run this script.

# Constant Defintiions

export QMAJ=17.1
export QMIN=590
export QUARTUS=${HOME}/intelFPGA_lite/${QMAJ}/quartus

# Install apt dependencies

sudo apt-get update
sudo apt-get install bison flex libc6-i386 make wget

# Create download directory if it doesn't already exist

if [ ! -d download ]; then
  mkdir -p download
fi

# Download tar files and installers

if [ ! -f download/Quartus-lite-${QMAJ}.0.${QMIN}-linux.tar ]; then
  cd download
  wget http://download.altera.com/akdlm/software/acdsinst/${QMAJ}std/${QMIN}/ib_tar/Quartus-lite-${QMAJ}.0.${QMIN}-linux.tar
  wget http://download.altera.com/akdlm/software/acdsinst/${QMAJ}std/${QMIN}/ib_installers/SoCEDSSetup-${QMAJ}.0.${QMIN}-linux.run
  chmod 775 SoCEDSSetup-${QMAJ}.0.${QMIN}-linux.run
  cd ..
fi

# Install quartus tools
#   - Accept license terms
#   - Uncheck all but base quartus tools and cyclone V tools
#   - Install SoCEDS tools to same directory as quartus tools (add _lite)
# TODO:
#   - The SoCEDS install script will hang if this script runs it after quartus installation
#   - Regardless of how I've tried running it, it seems to hang on success
#   - Running embedded_command_shell.sh will cause this script to exit early when it finishes

if [ ! -d quartus ]; then
  mkdir -p quartus
  tar -xf download/Quartus-lite-${QMAJ}.0.${QMIN}-linux.tar -C quartus
  quartus/setup.sh
  download/SoCEDSSetup-${QMAJ}.0.${QMIN}-linux.run
  cd ${HOME}/intelFPGA_lite/${QMAJ}/embedded
  ./embedded_command_shell.sh
  cd -
fi

# Create the de10 directory if it doesn't already exist

if [ ! -d de10 ]; then
  cp -R ../../de10 .
fi

# Compile default bitstream to rbf
if [ ! -f data/soc_system.rbf ]; then
  cd de10
  ${QUARTUS}/sopc_builder/bin/qsys-generate soc_system.qsys --synthesis=VERILOG
  ${QUARTUS}/bin/quartus_map DE10_NANO_SoC_GHRD.qpf
  ${QUARTUS}/bin/quartus_fit DE10_NANO_SoC_GHRD.qpf
  ${QUARTUS}/bin/quartus_asm DE10_NANO_SoC_GHRD.qpf
  ${QUARTUS}/bin/quartus_cpf -c sof2rbf.cof
  mv output_files/DE10_NANO_SoC_GHRD.rbf ../data
  cd ..
fi

# Compile pre-loader
#   - File -> New HPS BSP
#   - Select DE10_NANO_SoC_GHRD/hps_isw_handoff/soc_system_hps_0 in ... menu
#   - ok
#   - check FAT SUPPORT
#   - generate
# TODO:
#   - Running bsp-editor will shut this script down early

if [ ! -f data/preloader-mkpimage.bin ]; then
  cd de10
  bsp-editor&
  cd software/spl_bsp
  make
  mv preloader-mkpimage.bin ../../../data
  cd ../../..
fi

# TODO: Generate device tree 
