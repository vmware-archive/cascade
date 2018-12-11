![alt text](LOGO.png "Cascade: A JIT Compiler for Verilog from VMware Research")
---
[![Build Status](https://img.shields.io/travis/com/vmware/cascade/master.svg?style=flat-square)](https://travis-ci.com/vmware/cascade)
[![Coverage Status](https://img.shields.io/codecov/c/github/vmware/cascade.svg?style=flat-square)](https://codecov.io/gh/vmware/cascade)

FPGAs can exceed the performance of general-purpose CPUs by several orders of magnitude and offer dramatically lower cost and time to market than ASICs. While the benefits are substantial, programming an FPGA can be an extremely slow process. Trivial programs can take several minutes to compile using a traditional compiler, and complex designs can take hours or longer. 

Cascade is a novel solution to this problem, the world's first just-in-time compiler for Verilog. Cascade executes code immediately in a software simulator, and performs compilation in the background. When compilation is finished, the code is moved into hardware, and from the user’s perspective it simply gets faster over time. Cascade's ability to move code back and forth between software and hardware also makes it the first platform to provide generic support for the execution of unsynthesizable Verilog from hardware. The effects are substantial. Cascade encourages more frequent compilation, reduces the time required for developers to produce working hardware designs, and transforms HDL development into something which closely resembes writing JavaScript or Python. It takes the first steps towards bridging the gap between programming software and programming hardware.

Index
=====
0. [Dependencies](#dependencies)
1. [Building Cascade](#building-cascade)
2. [Using Cascade](#using-cascade)
    1. [Command Line Interface](#command-line-interface)
    2. [Other Interfaces](#other-interfaces)
3. [Environments](#environments)
    1. [Minimal Environment](#minimal-environment)
    2. [Software Backend](#software-backend)
    3. [Hardware Backend](#hardware-backend)
    4. [JIT Backend](#jit-backend)
4. [Verilog Support](#verilog-support)
5. [Standard Library](#standard-library)
6. [Adding Support for New Backends](#adding-support-for-new-backends)
7. [FAQ](#faq)

Dependencies
=====
Cascade should build successfully on OSX and most Linux distributions. Third-party dependencies can be retrieved from the command line using either ```apt-get``` (Ubuntu), ```opkg``` (Angstrom), or ```port``` (OSX). Note that on most platforms, this will require administrator privileges.
```
*NIX $ sudo (apt-get|port) install ccache cmake flex bison libncurses-dev
```

Building Cascade
=====
1. Clone this repository (make sure to use the ```--recursive``` flag)
```
*NIX $ git clone --recursive https://github.com/vmware/cascade cascade
```

2. Build the code (this process has been tested on OSX 10.12/10.14, Ubuntu 16.04, and Angstrom v2017.12)
```
*NIX $ cd cascade/
*NIX $ make
```
If the build didn't succeed (probably because you didn't use the ```--recursive``` flag) try starting over with a fresh clone of the repository.

3. Check that the build succeeded (all tests should pass)
```
*NIX $ make test
```

Using Cascade
=====

### Command Line Interface

Start Cascade by typing
```
*NIX $ ./bin/cascade
```
This will place you in a Read-Evaluate-Print-Loop (REPL). Code which is typed here is appended to the source of the (initially empty) top-level module and evaluated immediately. Try defining a wire. You'll see the text ```ITEM OK``` to indicate that a new module item was added to the top-level module.
```verilog
>>> wire x;
ITEM OK
```
The verilog specification requires that code inside of ```initial``` blocks is executed exactly once when a program begins execution. Because Cascade provides a dynamic environment, we generalize the specification as follows: Code inside of ```initial``` blocks is executed exactly once immediately after it is compiled. Try printing the value of the wire you just defined. 
```verilog
>>> initial $display(x);
ITEM OK 
>>> 0
```
Now try printing a variable which hasn't been defined.
```verilog
>>> initial $display(y);
*** Typechecker Error:
  > In final line of user input:
    Referenece to unresolved identifier: y
```
Anything you enter into the REPL is lexed, parsed, type-checked, and compiled. If any part of this process fails, Cascade will produce an error message and the remainder of your text will be ignored. If you type multiple statements, anything which compiles successfully before the error is encountered cannot be undone. Below, ```x``` and ```y``` are declared successfully, but the redeclaration of ```x``` produces an error.
```verilog
>>> wire x,y,x;
ITEM OK
ITEM OK
>>> *** Typechecker Error:
  > In final line of user input:
    A variable named x already appears in this scope.
    Previous declaration appears in previous user input.
>>> initial $display(y);
ITEM OK
>>> 0
```
You can declare and instantiate modules from the REPL as well. Be sure to note however, that  declarations will be type-checked in the global declaration scope. Variables which you may have declared in the top-level module will not be visible here. It isn't until a module is instantiated that it can access program state.
```verilog
>>> module Foo(
  input wire x,
  output wire y 
);
  assign y = x;
endmodule
DECL OK
>>> wire q,r;
ITEM OK
ITEM OK
>>> Foo f(q,r);
ITEM OK
```
If you don't want to type your entire program into the REPL you can use the include statement, where ```path/``` is assumed to be relative to your current working directory.
```verilog
>>> include path/to/file.v;
```
If you'd like to use additional search paths, you can start Cascade using the ```-I``` flag and provide a list of colon-separated alternatives. Cascade will try each of these paths as a prefix, in order, until it finds a match.
```
*NIX $ ./bin/cascade -I path/to/dir1:path/to/dir2
```
Alternately, you can start Cascade with the ```-e``` flag and the name of a file to include.
```
*NIX $ ./bin/cascade -e path/to/file.v
```
Finally, Cascade will stop running whenever a program invokes the ```$finish``` task.
```verilog
>>> initial $finish;
ITEM OK
Goodbye!
```
You can also force a shutdown by typing ```Ctrl-C``` or ```Ctrl-D```.
```verilog
>>> module Foo(); wir... I give up... arg... ^C
```

### Other Interfaces
If you're absolutely fixated on performance at all costs, you can deactivate the REPL by running Cascade in batch mode.
```
*NIX $ ./bin/cascade --batch -e path/to/file.v
```

On the other hand, if you prefer a GUI, Cascade has a frontend which runs in the browser.
```
*NIX $ ./bin/cascade --ui web
>>> Running server out of /Users/you/Desktop/cascade/bin/../src/ui/web/
>>> Server started on port 11111
```
```
*NIX $ (firefox|chrome|...) localhost:11111
```
If something on your machine is using port 11111, you can request that Cascade use a different port using the ```web-ui-port``` flag.
```
*NIX $ ./bin/cascade --ui web --web-ui-port 22222
```

Environments
=====

### Minimal Environment
By default, Cascade is started in a minimal environment. You can invoke this behavior explicitly using the ```--march``` flag.
```
*NIX $ ./bin/cascade --march minimal
```
This environment declares the following module and instantiates it into the top-level module for you:
```verilog
module Clock(
  output wire val
);
endmodule

Clock clock;
```
This module represents the global clock. Its value flips between zero and one every cycle. Try typing the following (and remember that you can type Ctrl-C to quit):
```verilog
>>> always @(clock.val) $display(clock.val);
ITEM OK
0
1
0
1
...
```
This global clock can be used to implement sequential circuits, such as the barrel shifter shown below.
```verilog
>>> module BShift(
  input wire clk,
  output reg[7:0] val
);
  always @(posedge clk) begin
    val <= (val == 8'h80) ? 8'h01 : (val << 1);
  end
endmodule
DECL OK
>>> wire[7:0] x;
ITEM OK
>>> BShift bs(clock.val, x);
ITEM OK
```
Compared to a traditional compiler which assumes a fixed clock rate, Cascade's clock is *virtual*: the amount of time between ticks can vary over time, and is a function of both how large your program and is and how often and how much I/O it produces. This abstraction is the key mechanism by which Cascade is able to move programs between software and hardware without involving the user.  

### Software Backend
Up until this point the code we've looked at has been entirely compute-based. However most hardware programs involve the use of I/O peripherals. Before we get into real hardware, first try running Cascade's virtual software FPGA.
```
*NIX $ ./bin/sw_fpga
```
Cascade's virtual FPGA provides an ncurses GUI with four buttons, one reset, and eight leds. You can toggle the buttons using the ```1 2 3 4``` keys, toggle the reset using the ```r``` key, and shut down the virtual FPGA by typing ```q```. In order to write code which uses these peripherals, restart Cascade using the ```--march sw``` flag on the same machine as the virtual FPGA.
```
*NIX $ ./bin/cascade --march sw
```
Cascade will automatically detect the virtual FPGA and expose its peripherals as modules which are implicitly declared and instantiated in the top-level module:
```verilog
module Pad(
  output wire[3:0] val
);
endmodule
Pad pad();

module Reset(
  output wire val
);
endmodule
Reset reset();

module Led(
  output wire[7:0] val
);
endmodule
Led led();
```
Now try writing a simple program that connects the pads to the leds.
```verilog
>>> assign led.val = pad.val;
ITEM OK
```
Toggling the pads should now change the values of the leds for as long as Cascade is running.

### Hardware Backend
Cascade currently provides support for a single hardware backend: the [Terasic DE10 Nano SoC](https://www.terasic.com.tw/cgi-bin/page/archive.pl?Language=English&No=1046). When Cascade is run on the DE10's ARM core, instead of mapping compute and leds onto virtual components, it can map them directly onto a real FPGA. Try ssh'ing onto the ARM core ([see FAQ](#faq)), building Cascade, and starting it using the ```--march de10``` flag.
```
DE10 $ ./bin/cascade --march de10
```
Assuming Cascade is able to successfully connect to the FPGA fabric, you will be presented with a similar environment to the one described above. The only difference is that instead of a Reset module, Cascade will implicitly declare the following module, which represents the DE10's Arduino header.
```verilog
module GPIO(
  input wire[7:0] val
);
endmodule
GPIO gpio();
```
Try repeating the example from the previous section and watch real buttons toggle real leds.

### JIT Backend
For each of the ```--march``` flags described above, the core computation of a program would still be performed in software. Cascade's ```--march de10_jit``` backend is necessary for transitioning a program completely to hardware. Before restarting Cascade, you'll first need to install [Intel's Quartus Lite IDE](http://fpgasoftware.intel.com/?edition=lite) on a 64-bit Linux machine. Once you've done so, connect this machine to your DE10's USB Blaster II Port [See FAQ](#faq), and verify the connection by typing.
```
64-Bit LINUX $ <quartus/install/dir>/jtagconfig
```
You'll see something like the following. Take note of the string in square brackets.
```
1) DE-SoC [1-3]
  4BA00477     SOCVHPS
  02D020DD     5CSEBA6(.|ES)/5CSEMA6/..
```
You can now start Cascade's JIT server by typing the following, where the ```--usb``` command line argument is what appeared when you invoked ```jtagconfig```.
```
64-Bit LINUX $ ./bin/quartus_server --path <quartus/install/dir> --usb "[1-3]"
```
Now ssh back into the ARM core on your DE10, and restart cascade with a very long running program by typing.
```
DE10 $ ./bin/cascade --quartus_host <64-Bit LINUX IP> --march de10_jit -I data/benchmark/bitcoin -e bitcoin.v --profile_interval 10
```
Providing the ```--profile_interval``` flag will cause cascade to periodically (every 10s) print the current time and Cascade's virtual clock frequency. Over time as the JIT compilation runs to completion, and the program transitions from software to hardware, you should see this value transition from O(10 KHz) to O(10 MHz). If at any point you modify a program which is mid-compilation, that compilation will be aborted. Modifying a program which has already transitioned to hardware will cause its execution to transition back to software while the new compilation runs to completion.

Verilog Support
=====
Cascade currently supports a large --- but certainly not complete --- subset of the Verilog 2005 Standard. The following partial list should give a good impression of what Cascade is capable of.

| Feature Class         | Feature                   | Supported | In Progress | Will Not Support | 
|:----------------------|:--------------------------|:---------:|:-----------:|:----------------:|
| Primitive Types       | Net Declarations          |  x        |             |                  |
|                       | Reg Declarations          |  x        |             |                  |
|                       | Integer Declarations      |  x        |             |                  |
|                       | Real Declarations         |           | x           |                  |
|                       | Time Declarations         |           | x           |                  |
|                       | Realtime Declarations     |           | x           |                  |
|                       | Array Declarations        |  x        |             |                  |
| Expressions           | Arithmetic Operators      |  x        |             |                  |
|                       | Bitwise Operators         |  x        |             |                  |
|                       | Logical Operators         |  x        |             |                  |
|                       | Concatentation Operators  |  x        |             |                  |
|                       | Conditional Operators     |  x        |             |                  |
|                       | Bit/Part Select           |  x        |             |                  |
|                       | Strings                   |           | x           |                  |
|                       | Real Constants            |           | x           |                  |
| Parameters            | Parameter Declarations    |  x        |             |                  |
|                       | Localparam Declarations   |  x        |             |                  |
|                       | Defparam Statements       |           |             | x                |
| Module Declarations   | Input Ports               |  x        |             |                  |
|                       | Output Ports              |  x        |             |                  |
|                       | Inout Ports               |           | x           |                  |
| Module Instantiations | Named Parameter Binding   |  x        |             |                  |
|                       | Ordered Parameter Binding |  x        |             |                  |
|                       | Named Port Binding        |  x        |             |                  |
|                       | Ordered Port Binding      |  x        |             |                  |
|                       | Instantiation Arrays      |           | x           |                  |
| Generate Constructs   | Genvar Declarations       |  x        |             |                  |
|                       | Case Generate Constructs  |  x        |             |                  |
|                       | If Generate Constructs    |  x        |             |                  |
|                       | Loop Generate Constructs  |  x        |             |                  |
| System Tasks          | Display Statements        |  x        |             |                  |
|                       | Finish Statements         |  x        |             |                  |
|                       | Write Statements          |  x        |             |                  |
|                       | Monitor Statements        |           | x           |                  |

Standard Library
=====
Cascade's Clock, along with the modules which are implicitly declared when cascade is run with the ```--march de10``` or ```--march sw``` flags, are part of its Standard Library. The complete set of I/O peripherals in the Standard Library, along with the ```march``` environments in which they are supported, is shown below.

| Component | minimal | sw | de10 | de10_jit |
|:----------|:-------:|:--:|:----:|:--------:|
| Clock     | x       | x  | x    | x        |
| Led       |         | x  | x    | x        |
| Pad       |         | x  | x    | x        |
| Reset     |         | x  |      |          |
| GPIO      |         |    | x    | x        |

Cascade's Standard Library also impliclty declares two reusable data-structures for communicating back and forth between hardware and software, a memory and a FIFO queue. In contrast to the modules described above, these modules are not implicitly instantiated. The user may instead instantiate as many as they like.

Cascade memories are dual-port read, single-port write. The declaration provided by the Standard Library is shown below.
```verilog
module Memory#(
  parameter ADDR_SIZE = 4,           // Address bit-width: A memory will have 2^ADDR_SIZE elements
  parameter BYTE_SIZE = 8            // Value bit-width: The value at each address will have BYTE_SIZE bits
)(
  input  wire clock,                 // Write data is latched on the posedge of this signal
  input  wire wen,                   // Assert to latch write data
  input  wire[ADDR_SIZE-1:0] raddr1, // Address to read data from
  output wire[BYTE_SIZE-1:0] rdata1, // The value at raddr1, available this clock cycle
  input  wire[ADDR_SIZE-1:0] raddr2, // Ditto
  output wire[BYTE_SIZE-1:0] rdata2, // Ditto
  input  wire[ADDR_SIZE-1:0] waddr,  // Address to write data to
  input  wire[BYTE_SIZE-1:0] wdata   // The value to write to waddr at the posedge of clock
);
```

When instantiating a memory, an optional annotation may be provided as well.
```verilog
(*__file="path/to/file.mem"*)
Memory#(4,8) mem(/* ... */);
```
This annotation tells Cascade that the initial values for the memory should be read from ```path/to/file.mem```, and that when Cascade finishes execution the state of the memory should be written back to that file. If the file does not exist when the module is instantiated, Cascade will initialize the memory to all zeros. As with include statements, Cascade will attempt to resolve the ```__file``` annotation relative to the paths provided by the ```-I``` flag.  

A memory file is expected to contain whitespace separated hexadecimal values, one for each address. For the memory instiated above (consisting of 2^4 8-bit values), an well-formed memory file might contain the following.
```
0 fc 10 6
1 2 3 4 ff fe fd fc
c d
a
b
```

Cascade FIFOs provide clocked read and write access to an arbitrary depth first-in-first-out queue. The declaration provided by the standard library is shown below.
```verilog
module Fifo#(
  parameter DEPTH = 8,              // The maximum number of elements that the fifo can hold
  parameter BYTE_SIZE = 8           // Each element in the fifo will have BYTE_SIZE bits
)(
  input  wire clock,                // Reads and writes happen on the posedge of this signal
  input  wire rreq,                 // Assert to pop a value from the fifo 
                                    // Pushing/popping at the same time, or reading an empty fifo is undefined
  output wire[BYTE_SIZE-1:0] rdata, // The value that was popped on the last posedge of clock
  input  wire wreq,                 // Assert to push a value onto the fifo
                                    // Pushing/popping at the same time, or writing a full fifo is undefined
  input  wire[BYTE_SIZE-1:0] wdata, // The value to push at the next posedge of clock
  output wire empty,                // Is this fifo currently empty?
  output wire full                  // Is this fifo currently full?
);
```
When instantiating a fifo, an optional set of annotations may be provided as well.
```verilog
(*__file="path/to/file.mem", __count=8*)
Fifo#(8,8) fifo(/* ... */);
```
The ```__file``` annotation is similar to the one described above. If provided, Cascade will attempt to initialize the FIFO with values drawn from this file. The file format is the same. If the file contains more values than the maximum depth of the FIFO, Cascade will automatically push the next value from the file as soon as there is space (```full``` reports false). This process will continue until the end-of-file is reached, and then repeat up to ```__count``` times before no longer attempting to push values into the FIFO. Values are not written back to ```__file``` when Cascade finishes execution, and it is an error to instantiate a FIFO with an unresolvable ```__file``` annotation.

Adding Support for New Backends
=====
(Coming soon.)

FAQ
====

#### Flex fails during build with an error related to ```yyin.rdbuf(std::cin.rdbuf())``` on OSX.
This has to do with the version of flex that you're using. Some versions of port will install an older version. Try using the version of flex provided by XCode in ```/usr/bin/flex```.

#### Bison fails during build with an error related to ```too many arguments provided to function-like macro``` on OSX.
(See issue #33) This has to do with the version of bison that you're using. This behavior has so far been observed with bison 3.2.x. Versions 3.0.4 or 3.1 both appear to work.

#### How do I ssh into the DE10's ARM core using a USB cable?
(Coming soon.)

#### How do I configure the DE10's USB Blaster II Programming Cable in a Linux Environment?
(Coming soon.)

#### Cascade emits strange warnings whenever I declare a module.
Module declarations are typechecked in their own scope, separate from the rest of the program. While this allows Cascade to catch many errors at declaration-time, there are some properties of Verilog programs which can only be verified at instantiation-time. If Cascade emits a warning, it is generally because it cannot statically prove that the module you just declared will instantiate correctly in every possible program context.  

#### Why does cascade warn that ```x``` is undeclared when I declare ```Foo```, but not when I instantiate it (Part 1)?
```verilog
localparam x = 0;
module Foo();
  wire q = x;
endmodule
Foo f();
```
The local parameter ```x``` was declared in the root module, and the module ```Foo``` was declared in its own scope. In general, there is no way for Cascade to guarantee that you will instantiate ```Foo``` in a context where all of its declaration-time unresolved variables will be resolvable. In this case, it is statically provable, but Cascade doesn't currently know how to do so. When ```Foo``` is instantiated, Cascade can verify that there is a variable named ```p``` which is reachable from the scope in which ```f``` appears. No further warnings or errors are necessary. Here is a more general example:
```verilog
module Foo();
  assign x.y.z = 1;
endmodule

// ...
begin : x
  begin : y
    reg z;
  end
end
// ...
Foo f(); // This instantiation will succeed because a variable named z
         // is reachable through the hierarchical name x.y.z from f.
         
// ...
begin : q
  reg r;
end
// ...
Foo f(); // This instantiation will fail because the only variable
         // reachable from f is q.r.
```

#### Why does cascade warn that ```x``` is undeclared when I declare ```Foo```, but not when I instantiate it (Part 2)?
```verilog
module #(parameter N) Foo();
  genvar i;
  for (i = 0; i < N; i=i+1) begin : GEN
    reg x;
  end
  wire q = GEN[5].x;
endmodule
Foo#(8) f();
```
The register ```x``` was declared in a loop generate construct with bounds determined by a parameter. In general, there is no way for Cascade to guarantee that you will instantiate ```Foo``` with a parameter binding such that all of its declaration-time unresolved variables are resolvable. When ```Foo``` is instantiated with ```N=8```, Cascade can verify that there is a variable named ```GEN[5].x```. No further warnings or errors are necessary.

More generally, Cascade will defer typechecking for code that appears inside of generate constructs until instantiation-time.

#### I get it, but it seems like there's something about pretty much every module declaration that Cascade can't prove.
The truth hurts. If you'd like to disable warnings you can type.
```
$ ./bin/cascade --disable_warnings
```
