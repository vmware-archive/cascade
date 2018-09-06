// Start Frame
localparam[31:0] start  = 32'h00000000;
// LED Frame (values set by pads)
wire[31:0] frame = { 
  3'b111,                            // header
  (pad.val[0] ? 5'b01111    : 5'b1), // brightness
  (pad.val[1] ? 8'b11111111 : 8'b0), // blue
  (pad.val[2] ? 8'b11111111 : 8'b0), // green // BUG! should be pad.val[2]
  (pad.val[3] ? 8'b11111111 : 8'b0)  // red   // BUG! should be pad.val[3]
};
// End Frame
localparam[31:0] finish = 32'hffffffff;

// Program state
reg[2367:0] buffer = 0;
reg[31:0] count = 0;

always @(posedge clock.val) begin
  // Counter variable; one tick for each bit in packet format
  if (count == 2368) begin
    count <= 0;
  end else begin
    count <= count + 1;
  end

  // Grab buffer every time count loops back to zero
  // Otherwise shift results by one bit
  if (count == 0) begin
    buffer <= {start, {72 {frame}}, finish};
  end else begin
    buffer <= (buffer << 1);
  end
end

// DEBUG: Print packet whenever inputs change
always @(pad.val) begin
  $display("New input from user: ");
  $display("  [0x32] 72x[111 %b %b %b %b] [1x32]", 
    frame[28:24], frame[23:16], frame[15:8], frame[7:0]);
end

// Connect cascade clock to clock line
// Connect most-significant packet bit to data line
assign gpio.val[0] = buffer[2367];
assign gpio.val[1] = clock.val;
