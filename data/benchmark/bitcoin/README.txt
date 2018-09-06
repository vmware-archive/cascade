README:

This is a copy of the bitcoin_9.v test from the bitcoin test suite. It is
configured with a difficulty of 19 bits, and should take approximately 30
minutes to complete when cascade is run in pure software mode on a macbook
(likely longer when run on a de10 arm core). This is definitely long enough for
at least one full pass of hardware compilation to complete before the program
terminates. This is a little bit on the long side, but the next lowest difficulty
(18 bits) takes only 4 minutes to run to completion.

As of this revision, setting the difficulty to 18 is sufficient for jit
compilation to run on the DE10.

EXPECTED OUTPUT @ 18: 62e7 62ec
