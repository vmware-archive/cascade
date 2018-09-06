# Compute the sum $1 = 0:9. Places the result in $2.

xor $3, $3, $3   
addi $3, $3, 10 # $3 = 10

xor $1, $1, $1  # $1 = 0
xor $2, $2, $2  # $2 = 0
loop:
  beq $1, $3, done # while ($1 != $3)
  add $2, $2, $1   # $2 += $1
  addi $1, $1, 1   # ++$1
  j loop
done:

halt # assert($2 == 45)
