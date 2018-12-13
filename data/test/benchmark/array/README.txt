This benchmark is meant to stress cascade's ability to perform
multi-dimensional array dereferences in both software and hardware. The length
of the simulation is controlled by the localparam declaration at the top of
array.v. Larger values of W will produce longer simulations.  The program will
print out the number of virtual clock cycles required for it to finish. The
expected number of cycles is given by the formula shown below.

 W : 2^(4*W) + 1
---------------------------------
10 : 
 9 : 
 8 : 
 7 : 268435457 2^28 + 1
 6 : 16777217  2^24 + 1
 5 : 1048577   2^20 + 1
 4 : 65537     2^16 + 1
 3 : 4097      2^12 + 1
 2 : 257       2^ 8 + 1
 1 : n/a
 0 : n/a
