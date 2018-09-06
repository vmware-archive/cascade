![alt text](LOGO.png "Cascade: A JIT Compiler for Verilog")

### Dependencies
This project *should* build successfully on both OSX and Ubuntu. Third-party dependencies can be retrieved with either ```apt-get``` (Ubuntu) or ```port``` (OSX).

1. Lexing and Parsing
```
$ sudo (apt-get|port) install flex bison libgmp3-dev
```
2. Build
```
$ sudo (apt-get|port) install ccache
```
3. Additional Dependencies?
```
$ sudo (apt-get|port) install ...
```

### Build
1. Clone source (make sure to use the ```--recursive``` flag)
```
$ git clone --recursive https://github.com/jitfpga/fpga
```
2. Build
```
$ make
```
3. The build didn't succeed (because you didn't use the ```--recursive``` flag)
```
$ make submodule
$ make
```
4. Test
```
$ make check
```

### Run
#### The Basics
You can start the system by typing:
```
$ ./bin/fpga
```
This will place you in a REPL loop. Code which is typed here is appended to the source of the (initially empty) top-level module, JIT-compiled, and executed immediately. Try defining a wire. If things go well, you'll see the text ```OK```.
```verilog
>>> wire x;
OK
```
The verilog specification requires that code inside of ```initial``` blocks is executed exactly once at the beginning of the simulation. Because this is a JIT-compiler, we generalize this specification as follows: Code inside of ```initial``` blocks is executed exactly once at the beginning of the first time step after it is compiled. Try printing the value of the wire you just defined. 
```verilog
>>> initial $display(x);
OK 
>>> 0
```
Now try printing a value that isn't in scope:
```verilog
>>> initial $display(y);
*** Checker Error:
  Reference to an undefined identifier!
  y
>>>
```
In general, anything you type from the REPL is lexed, parsed, type-checked, and compiled. If any part of this process fails, you will see an error message and the program will be left unmodified. However, if you type multiple statements and only the last fails, the statements which were compiled successfully cannot be undone. Below, ```x``` and ```y``` are declared successfully, but the redeclaration of ```x``` produces an error. This error does not change the fact that ```x``` and ```y``` were successfully introduced into the program.
```verilog
>>> wire x,y,x;
OK
OK
*** Checker Error:
  A variable with this name already appears in this scope!
  x
>>> initial $display(x);
OK
>>> 0
```
You can declare modules from the REPL, too.
```verilog
>>> module foo(x,y); 
input wire x; 
output wire y;
assign y = x;
endmodule
OK
```
Assuming they lex, parse, and typecheck, you can instantiate them as well.
```verilog
>>> wire x,y;
OK
OK
>>> foo f(x,y);
OK
```
If you don't feel like typing everything from scratch, you can include a file by typing the following, where path is assumed to be relative to the current working directory.
```verilog
>>> include path/to/file.v;
```
If you'd like to introduce additional search paths, you can restart the tool using the ```--include_dir``` flag and providing a list of colon-separated paths.
```
$ ./bin/fpga --include_dir path/to/dir1:path/to/dir2
```
And if you'd like to run the tool completely free of interaction, you can restart it with the ```-e``` flag and the name of a file to include.
```
$ ./bin/fpga -e path/to/file.v
```
Finally, a program can shut down the tool at any time by invoking the ```$finish``` task:
```verilog
>>> initial $finish;
OK
Goodbye!
```
Or you can do it yourself by typing ```Ctrl-C```.
```verilog
>>> module foo(); wir... I give up... arg... ^C
```

#### Minimal Environment
By default, the system is started in a minimal environment. You can invoke this behavior explicitly by typing:
```
$ ./bin/fpga --march minimal
```
This implicitly declares a module with the following definition and instantiates it into the top-level module for you:
```verilog
module Clock(val);
  output wire val;
endmodule
Clock clock;
```
This module represents the runtime's virtual clock. Its value flips between zero and one every virtual clock cycle. Try typing the following (and remember that you can type Ctrl-C to quit):
```verilog
>>> always @(clock.val) $display(clock.val);
OK
0
1
0
1
...
```
#### Software Target
If you want more than just a clock, you can try booting up a software fpga by typing:
```
$ ./bin/sw_target
```
This program provides an ncurses gui with four buttons, one reset, and eight leds. You can toggle the buttons using the ```1 2 3 4``` keys and the reset using the ```r``` key. You can shut the fpga down by typing ```q```. In order to communicate with this target, restart the system by typing:
```
$ ./bin/fpga --march sw
```
Assuming the system is able to successfully connect to the software fpga, these new assets will be exposed as additional modules which are implicitly declared and instantiated in the top-level module:
```verilog
module Pad(val);
  output wire[3:0] val;
endmodule
Pad pad();

module Reset(val);
  output wire val;
endmodule
Reset reset();

module Led(val);
  output wire[7:0] val;
endmodule
Led led();
```
Now try writing a simple program that connects the pads to the leds.
```verilog
>>> assign led.val = pad.val;
OK
```
Toggling the pads should now change the values of the leds.

#### de10 Target
All of the functionality described above is also supported on the terasic de10 soc. Except that instead of mapping compute and leds to virtual components, the system maps them directly onto a real fpga. Try booting up the de10 target by ssh'ing onto the ARM core on the de10 and typing.
```
$ ./bin/de10_target
```
In order to communicate with this target, restart the system by typing:
```
$ ./bin/fpga --march de10
```
Assuming the system is able to successfully connect to the de10, you will be presented with the same environment as above. Try repeating the example and watch real buttons toggle real leds.

#### Standard Library
In general, assets such as Clocks, Pads, and Leds can be thought of as standard peripheral components with a *more or less* well-defined interface. Currently, most assets are supported on most targets. However, as the standard library grows and we introduce support for more targets, this graph may become sparser:

|Asset|minimal|sw |de10|
|---|---|---|---|
|Clock|yes|yes|yes|
|Pad|no|yes|yes|
|Led|no|yes|yes|
|Reset|no|yes|no|

### DE10 Programming Notes
This process has only been verified on Ubuntu.

1. Download and install the Quartus Toolchain from www.altera.com/downloads/download-center.html
2. Configure the USB-Blaster driver by typing (source https://ubuntuforums.org/showthread.php?t=1441742)
```
$ sudo mount --bind /dev/bus /proc/bus
$ sudo ln -s /sys/kernel/debug/usb/devices /proc/bus/usb/devices
$ sudo <quartus_directory>/bin/jtagd
$ sudo <quartus_directory>/bin/jtagconfig
$ <quartus_directory>/bin/quartus
```
3. Create a quartus project ```<proj>.qpf``` with a minimal top-level module and constraint file.
4. Build a project and program the DE10 from the command line
```
$ <quartus_dir>/quartus/bin/quartus_map <proj>.qpf
$ <quartus_dir>/quartus/bin/quartus_fit <proj>.qpf
$ <quartus_dir>/quartus/bin/quartus_asm <proj>.qpf
$ <quartus_dir>/quartus/bin/quartus_pgm -c "DE-SoC [1-2]" --mode JTAG -o "P;<proj>.sof@2"
```

### Git Submodule Notes
A really helpful cheatsheet (https://medium.com/@porteneuve/mastering-git-submodules-34c65e940407)
