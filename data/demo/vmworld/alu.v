// This program performs basic alu operations. It prints the sum of the four pads on the
// first bank of leds, the product on the second, the logical and on the third, and the
// logical or on the fourth. This program is compatible with the vfpga_2345 march file.

assign led.val[0+:8]  = pad.val[0+:4] + pad.val[4+:4] + pad.val[8+:4] + pad.val[12+:4];
assign led.val[8+:8]  = pad.val[0+:4] * pad.val[4+:4] * pad.val[8+:4] * pad.val[12+:4];
assign led.val[16+:8] = pad.val[0+:4] & pad.val[4+:4] & pad.val[8+:4] & pad.val[12+:4];
assign led.val[24+:8] = pad.val[0+:4] | pad.val[4+:4] | pad.val[8+:4] | pad.val[12+:4];
