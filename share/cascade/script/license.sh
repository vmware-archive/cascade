#/bin/sh

# Top-Level Directories
DIRS="src test tools"
# Sub-Directories to Ignore
IGN="src/target/core/de10/fpga"
# File Types
EXTS=".h .cc .v"

for dir in $DIRS
do
  for ext in $EXTS 
  do
    for file in `find $dir -type f | grep $ext$`
    do
      for ign in $IGN 
      do
        if [[ $file != $IGN* ]]
        then
          echo "// Copyright 2017-2020 VMware, Inc." >> TEMP
          echo "// SPDX-License-Identifier: BSD-2-Clause" >> TEMP
          echo "//" >> TEMP
          echo "// The BSD-2 license (the "License") set forth below applies to all parts of the" >> TEMP
          echo "// Cascade project.  You may not use this file except in compliance with the" >> TEMP
          echo "// License." >> TEMP 
          echo "//" >> TEMP
          echo "// BSD-2 License" >> TEMP
          echo "//" >> TEMP
          echo "// Redistribution and use in source and binary forms, with or without" >> TEMP
          echo "// modification, are permitted provided that the following conditions are met:" >> TEMP
          echo "//" >> TEMP
          echo "// 1. Redistributions of source code must retain the above copyright notice, this" >> TEMP
          echo "// list of conditions and the following disclaimer." >> TEMP
          echo "//" >> TEMP
          echo "// 2. Redistributions in binary form must reproduce the above copyright notice," >> TEMP
          echo "// this list of conditions and the following disclaimer in the documentation" >> TEMP
          echo "// and/or other materials provided with the distribution." >> TEMP
          echo "//" >> TEMP
          echo "// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND" >> TEMP
          echo "// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED" >> TEMP
          echo "// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE" >> TEMP
          echo "// DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE" >> TEMP
          echo "// FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL" >> TEMP
          echo "// DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR" >> TEMP
          echo "// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER" >> TEMP
          echo "// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY," >> TEMP
          echo "// OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE" >> TEMP
          echo "// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE." >> TEMP
          echo "" >> TEMP

          line1=`head $file`
          if [[ $line1 == "// Copyright"* ]]
          then 
            tail -n+31 $file >> TEMP
          else
            cat $file >> TEMP
          fi

          mv TEMP $file
        fi
      done
    done
  done
done
