This is a copy of the disjunct test from the regex test suite.  It is
configured to loop over the input file 256 times, which should take
approximately 16 minutes to complete when cascade is run in pure software mode
on a macbook (likely longer when run on a de10 arm core). Ideally this is long
enough that when cascade is run with jit compilation enabled, at least one full
pass of hardware compilation will have time to complete before the program
terminates.

As of this revision, setting the number of loop iterations to 8 or above is
sufficient for jit compilation to run to completion on the DE10. 

EXPECTED RESULT: <loop iterations> * 424
