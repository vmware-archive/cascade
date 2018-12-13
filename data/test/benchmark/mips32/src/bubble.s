### CONSTANTS
xor $15, $15, $15  
addi $15, $15, 2048  # $15 (size) = The number of words in memory
xor $14, $14, $14
addi $14, $14, 2047  # $14 = size - 1

### INITIALIZE MEMORY
xor $1, $1, $1           # $1 (idx) = 0
init:                    
  beq $1, $15, done_init # while (idx != size)
  sub $2, $15, $1        #   $2 = size - idx 
  sll $3, $1, 2          #   $3 = 4*idx
  sw $2, 0($3)           #   mem[4*idx] = size - idx
  addi $1, $1, 1         #   ++idx
  j init
done_init:

### BUBBLE SORT
xor $13, $13, $13 
addi $13, $13, 1 # $13 (swapped) = true
xor $12, $12, $12 # $12 (false) = 0

loop:
  beq $13, $12, done_loop   # while (swapped)
  xor $13, $13, $13         #   swapped = false;

  xor $1, $1, $1            #   idx = 0;
  inner: 
    beq $1, $14, done_inner  #   while (idx != size-1)
    sll $2, $1, 2            #     $2 = 4*idx
    lw $3, 0($2)             #     $3 = mem[i]
    addi $4, $2, 4           #     $4 = 4*idx + 4
    lw $5, 0($4)             #     $5 = mem[j]

    slt $6, $5, $3           #     
    beq $6, $12, no_swap     #     if (mem[i] > mem[j]) 
      sw $5, 0($2)           #       
      sw $3, 0($4)           #       swap(mem[i], mem[j]) 
      addi $13, $13, 1       #       swapped = true
    no_swap:

    addi $1, $1, 1           #     ++idx
    j inner
  done_inner:

  j loop
done_loop:

### EPILOGUE
xor $2, $2, $2 # $2 = 0
lw $2, 0($2)   # $2 = mem[0]
halt           # assert($2 == 1)
