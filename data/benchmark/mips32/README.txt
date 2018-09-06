This is a copy of the bubble test from the mips32 test suite. It is configured
to run on an input array of length 2048 which should take approximately 16
minutes to sort when cascade is run in software-only mode on a macbook (likely
longer when run on a de10 arm core). Ideally this is long enough that when
cascade is run with jit enabled, at least one full pass of hardware compilation
will have time to complete before the program terminates.

If you'd like to change the configuration of this benchmark without rerunning
the assembler in the test directory, you can edit imem.mem in this directory as
follows.  Change the last four hex digits on line (2) to the desired array
length and the last four hex digits on line (4) to the desired array length-1.

As of this revision, setting array length to 0x0200 is sufficient for
jit compilation to run to completion on the DE10.

EXPECTED RESULT: 1
