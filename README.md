![alt text](LOGO.png "Cascade: A JIT Compiler for Verilog")

(This project just transitioned to open source. Please bear with us in the next few weeks this document evolves.)

Index
=====
0. [Dependencies](#dependencies)
1. [Building Cascade](#building-cascade)
2. [Using Cascade](#using-cascade)
    1. [The Basics](#the-basics)
    2. [Alternate UIs](#alternate-uis)
    3. [Default Environment](#default-environment)
    4. [Software Backend](#software-backend)
    5. [DE10 Backend](#de10-backend)
3. [Verilog Support](#verilog-support)
4. [Standard Library](#standard-library)

Dependencies
=====
Cascade should build successfully on both OSX and Ubuntu. Third-party dependencies can be retrieved using either ```apt-get``` (Ubuntu) or ```port``` (OSX). Please contact the developers if you discover a dependency which is not shown below.

```
$ sudo (apt-get|port) install ccache cmake flex bison libgmp3-dev 
```

Building Cascade
=====
1. Clone this repository (make sure to use the ```--recursive``` flag)
```
$ git clone --recursive https://github.com/vmware/cascade cascade
```
2. Build
```
$ cd cascade/
$ make
```
If the build didn't succeed (because you didn't use the ```--recursive``` flag)
```
$ rm -rf ext/*
$ make submodule
$ make
```
3. Check that the build succeeded (all tests should pass)
```
$ make check
```

Using Cascade
=====

### The Basics

Start Cascade by typing
```
$ ./bin/cascade
```
This will place you in a Read-Evaluate-Print-Loop (REPL). Code which is typed here is appended to the source of the (initially empty) top-level module and executed immediately. Try defining a wire. You'll see the text ```ITEM OK``` to indicate that a new module item was added to the top-level module.
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
*** Referenece to unresolved identifier >>> y <<< 
>>>
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
You can declare and instantiate modules from the REPL as well.
```verilog
>>> module foo(
  input wire x,
  output wire y 
);
  assign y = x;
endmodule
DECL OK
>>> wire q,r;
ITEM OK
ITEM OK
>>> foo f(q,r);
ITEM OK
```
If you don't want to type your entire program into the REPL you can use the include statement, where ```path/``` is assumed to be relative to your current working directory.
```verilog
>>> include path/to/file.v;
```
If you'd like to use additional search paths, you can start Cascade using the ```-I``` flag and provide a list of colon-separated alternatives. Cascade will try each of these paths as a prefix, in order, until it finds a match.
```
$ ./bin/cascade -I path/to/dir1:path/to/dir2
```
Alternately, you can start Cascade with the ```-e``` flag and the name of a file to include.
```
$ ./bin/cascade -e path/to/file.v
```
Finally, Cascade will stop running whenever a program invokes the ```$finish``` task.
```verilog
>>> initial $finish;
ITEM OK
Goodbye!
```
You can also force a shutdown by typing ```Ctrl-C``` or ```Ctrl-D```.
```verilog
>>> module foo(); wir... I give up... arg... ^C
```

### Alternate UIs
If you're absolutely fixated on performance at all costs, you can deactivate the REPL by running Cascade in batch mode.
```
$ ./bin/cascade --batch -e path/to/file.v
```

If you prefer a GUI, cascade has a frontend which runs in the browser.
```
$ ./bin/cascade --ui web
>>> Running server out of /Users/you/Desktop/cascade/bin/../src/ui/web/
>>> Server started on port 11111
```
```
$ (firefox|chrome|...) localhost:11111
```
If something on your machine is using port 11111, you can specify an alternate using the ```web-ui-port``` flag.
```
$ ./bin/cascade --ui web --web-ui-port 22222
```

### Default Environment
By default, Cascade is started in a minimal environment. You can invoke this behavior explicitly using the ```--march``` flag.
```
$ ./bin/cascade --march minimal
```
This environment declares the following module and instantiates it into the top-level module for you:
```verilog
module Clock(
  output wire val
);
endmodule

Clock clock;
```
This module represents Cascade's clock. Its value flips between zero and one every virtual clock cycle. Try typing the following (and remember that you can type Ctrl-C to quit):
```verilog
>>> always @(clock.val) $display(clock.val);
ITEM OK
0
1
0
1
...
```

### Software Backend
If you'd like to write a program with additional peripherals, you can try using Cascade's virtual FPGA.
```
$ ./bin/sw_fpga
```
Cascade's virtual FPGA provides an ncurses GUI with four buttons, one reset, and eight leds. You can toggle the buttons using the ```1 2 3 4``` keys, toggle the reset using the ```r``` key, and shut down the virtual FPGA by typing ```q```. To use this backend, restart Cascade by typing:
```
$ ./bin/cascade --march sw
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
Toggling the pads should change the values of the leds.

### DE10 Backend
All of the functionality described above is also supported on the terasic de10 soc. Except that instead of mapping compute and leds to virtual components, the system maps them directly onto a real fpga. Try booting up the de10 target by ssh'ing onto the ARM core on the de10 and typing.
```
$ ./bin/de10_target
```
In order to communicate with this target, restart the system by typing:
```
$ ./bin/fpga --march de10
```
Assuming the system is able to successfully connect to the de10, you will be presented with the same environment as above. Try repeating the example and watch real buttons toggle real leds.

### Verilog Support
(Coming soon.)

### Standard Library
(Coming soon.)
