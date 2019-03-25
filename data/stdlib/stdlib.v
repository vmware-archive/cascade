///////////////////////////////////////////////////////////////////////////////
//
// This file contains declarations for the cascade standard library.  The names
// of these modules are reserved by cascade and cannot be overriden by user
// programs.  Target-specific backend implementations are expected to provide a
// march file which instantiates whichever of these modules that backend
// supports and to provide appropriate annotations for those instantiations.
// At a minimum, a target-specific implementation must instantiate both the
// Root and the global clock.
//
// All of the modules in the standard library support the following
// annotations.  Additional module-specific annotations are discussed below.
//
// __target = "..." 
//   Required. Tells the compiler class which core compiler to use. Providing
//   a second target, separated by a colon, tells the compiler class which
//   core compiler to use during second pass compilation.
// __loc = "..."
//   Optional. Tells the compiler class which interface compiler to use.  If
//   not provided, defaults to "runtime", ie in the same process space as the
//   runtime. Providing a second location, separated by a colon, tells the
//   compiler class which core compiler to use during second pass compilation.
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Minimal Implementation:
///////////////////////////////////////////////////////////////////////////////

// The top-level module. Evaluated module items are inserted here.
(*__std="logic", __loc="runtime", __target="sw"*)
module Root(); 
endmodule

// The top-level virtual clock.
(*__std="clock"*)
module Clock(
  output wire val
);

  // Possible Implementation:
  //
  // output reg val = 0;
  // always @(val) begin
  //  #1; val <= ~val;  
  // end

endmodule

///////////////////////////////////////////////////////////////////////////////
// Target-Specific Components:
///////////////////////////////////////////////////////////////////////////////

// The top-level reset signal
(*__std="reset"*)
module Reset(
  output wire val
);

  // Possible Implementation:
  //
  // output wire val = <reset pin>;

endmodule

// An arbitrary width source of binary inputs
(*__std="pad"*)
module Pad#(
  parameter WIDTH = 4
)(
  output wire[WIDTH-1:0] val
);

  // Possible Implementation:
  //
  // output wire[WIDTH-1:0] val = <pad pins>;

endmodule

// An arbitrary width source of binary outputs
(*__std="led"*)
module Led#(
  parameter WIDTH = 8
)(
  input wire[WIDTH-1:0] val
);

  // Possible Implementation:
  //
  // input wire[WIDTH-1:0] val;
  // assign <led pins> = val;

endmodule

// An arbitrary width source of binary outputs
(*__std="gpio"*)
module Gpio#(
  parameter WIDTH = 8
)(
  input wire[WIDTH-1:0] val
);

  // Possible Implementation:
  //
  // input wire[WIDTH-1:0] val;
  // assign <gpio pins> = val;

endmodule

///////////////////////////////////////////////////////////////////////////////
// Reusable Data-Structures:
//
// These abstracts are deprecated and should no longer be used!  The preferred
// method for performing file i/o is the file i/o family of system tasks.
//
///////////////////////////////////////////////////////////////////////////////

// An dual-port-read single-port-write memory with arbitrary size and byte
// width.  Supports additional annotations below:
// 
// __file = "..."
//   Optional. If specified the contents of this memory will be read/written
//   from/to this file when the program begins/finishes executing.
(*__std="memory",__target="sw",__loc="runtime"*)
module Memory#(
  parameter ADDR_SIZE = 4,
  parameter BYTE_SIZE = 8
)(
  input  wire clock,
  input  wire wen,
  input  wire[ADDR_SIZE-1:0] raddr1,
  output wire[BYTE_SIZE-1:0] rdata1,
  input  wire[ADDR_SIZE-1:0] raddr2,
  output wire[BYTE_SIZE-1:0] rdata2,
  input  wire[ADDR_SIZE-1:0] waddr,
  input  wire[BYTE_SIZE-1:0] wdata
);
endmodule

// A bounded-depth read/write fifo. Supports additional annotations below.
// Attempting to write a new value into a full fifo or to read a value from
// an empty fifo will result in undefined behavor.
//
// __file = "..."
//   Optional. If specified this fifo will be initialized with values taken
//   from this file. If there are more values in this file than the fifo's
//   maximum depth, this fifo will continue to pull values from the file until
//   it is exhausted.
// __count = "..."
//   Optional. If specifid along with __file, this fifo will be initialized
//   with 'count' copies of the data in __file.
(*__std="fifo",__target="sw",__loc="runtime"*)
module Fifo#(
  parameter DEPTH = 8,
  parameter BYTE_SIZE = 8
)(
  input  wire clock,
  input  wire rreq,
  output wire[BYTE_SIZE-1:0] rdata,
  input  wire wreq,
  input  wire[BYTE_SIZE-1:0] wdata,
  output wire empty,
  output wire full
);
endmodule
