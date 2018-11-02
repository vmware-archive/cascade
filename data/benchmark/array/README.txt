This benchmark is meant to stress cascade's ability to perform
multi-dimensional array dereferences in both software and hardware. The length
of the simulation is controlled by the localparam declaration at the top of
array.v. Larger values of W will produce longer simulations.  The program will
print out the number of virtual clock cycles required for it to finish. The
expected number of cycles is given by the formula shown below.

(2^W) * (2^(2*W)-1) + 1 cycles

W = 2 : 61
    3 : 505
    4 : 4081
    5 : 32737
    6 : 262081
    7 : 2097025
    8 : 16776961
    9 : 134217217
   10 : 1073740801
