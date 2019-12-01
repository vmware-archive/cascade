`ifndef __CASCADE_SHARE_STDLIB_STDLIB_V
`define __CASCADE_SHARE_STDLIB_STDLIB_V

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
//   Required. Tells the compiler class which core compiler to use. Providing a
//   second target, separated by a semi-colon, tells the compiler class which
//   core compiler to use during second pass compilation.
// __loc = "..."
//   Optional. Tells the compiler class which interface compiler to use.  If
//   not provided, defaults to "local", ie in the same process space as the
//   runtime. Providing a second location, separated by a semi-colon, tells the
//   compiler class which core compiler to use during second pass compilation.
//
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
// Minimal Implementation:
///////////////////////////////////////////////////////////////////////////////

// The top-level module. Evaluated module items are inserted here.
(*__std="logic", __loc="local", __target="sw"*)
module Root();
  localparam STDIN   = 32'h8000_0000;
  localparam STDOUT  = 32'h8000_0001;
  localparam STDERR  = 32'h8000_0002;
  localparam STDWARN = 32'h8000_0003;
  localparam STDINFO = 32'h8000_0004;
  localparam STDLOG  = 32'h8000_0005;
endmodule

// The top-level virtual clock.
(*__std="clock", __loc="local", __target="sw"*)
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

`endif
