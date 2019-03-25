This directory contains several compilations of regular expressions into
cascade compatible verilog programs. The expected result for running each
benchmark is a function of the number of times that it loops over the hex
source file.

FILE        : REGEX                : EXPECTED OUTPUT
----------------------------------------------------
word        : Achilles             : 423 * N
disjunct    : (Achilles)|(THE END) : 424 * N
