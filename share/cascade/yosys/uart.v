`ifndef UART_V
`define UART_V

`include "uart_rx.v"
`include "uart_tx.v"

module Uart#(
  parameter DIVIDER = 25000000/115200,
  parameter RSIZE = 4, 
  parameter WSIZE = 4  
)(
  input wire clk,

  output wire ready,
  input wire enable,

  input wire ftdi_to_fpga,
  output reg[8*RSIZE-1:0] rdata,

  output wire fpga_to_ftdi,
  input wire[8*WSIZE-1:0] wdata,
);

  wire[7:0] rx_data;
  wire rx_valid;
  UartRx#(
    .DIVIDER(DIVIDER)
  )rx(
    .clk(clk),
    .serial(ftdi_to_fpga),
    .data(rx_data),
    .valid(rx_valid)
  );

  reg[7:0] tx_data = 0;
  reg tx_enable = 0;
  wire tx_ready;
  UartTx#(
    .DIVIDER(DIVIDER)
  )tx(
    .clk(clk),
    .serial(fpga_to_ftdi),
    .enable(tx_enable),
    .data(tx_data),
    .ready(tx_ready)
  );

  reg[7:0] state = 0;
  always @(posedge clk) begin
    tx_enable <= 0;

    // Read phase
    if (state < RSIZE) begin
      if (rx_valid) begin
        state <= state + 1;
        rdata[8*state+:8] <= rx_data;
      end
    end

    // Wait for enable to begin write phase
    else if (state == RSIZE) begin
      if (enable) begin
        state <= state + 1;
        tx_data <= wdata[7:0]; 
        tx_enable <= 1;
      end
    end

    // Write phase
    else if (state > RSIZE) begin
      if (tx_ready) begin
        if (state < RSIZE+WSIZE) begin
          state <= state + 1;
          tx_data <= wdata[8*(state-RSIZE)+:8];
          tx_enable <= 1;
        end else begin
          state <= 0;
          tx_data <= 0;
          tx_enable <= 0;
        end
      end
    end
  end

  assign ready = (state == RSIZE);

endmodule

`endif
