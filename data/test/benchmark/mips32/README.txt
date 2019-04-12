This directory contains an implementation of the single-cycle mips32 processor
described in Computer Architecture a Quantitative Approach. The instruction
memories in this directory were created by assembling the source found in the
src/ directory.

Running any variant of the bubble program will produce the same expected
result, 1. The only difference is running time.

Beginning with 128 as an input size, we transition from growing the working set
to looping over the sorting task. This is to spare smaller targets which don't
have so large a working memory size.
