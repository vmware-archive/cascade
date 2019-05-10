`define ONE 1
`define TWO(x) 2
`define foo(x,y,z) (x*y+z)
`define FIN $finish; end

`define bar initial begin\
$write(     \
        `foo(3,`foo(3,`TWO(27),`ONE),1));  \
\
\
\
        \
`FIN

`bar
