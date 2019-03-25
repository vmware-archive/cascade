This directory contains a reference implementation of the needelman-wunsch
algorithm.  The code will compute the cost matrix for a set of inputs and
return the cost associated with the optimal matching. To test correctness, we
compute a checksum of this result over a large set of inputs and print the
result.

This implementation computes an optimal matching weight every cycle. If we're
performant on this code, we'll definitely be performant on code which is
pipelined.  Depending on input size, this code will return different values:

INPUT SIZE : CHECKSUM
---------------------
         2 : -722
         4 : -1126
         8 : -1440
        16 : 
