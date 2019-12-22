#!/bin/bash

# $1 = unique compilation name

cd $1

ujprog -b 3000000 root32.bit

cd -
