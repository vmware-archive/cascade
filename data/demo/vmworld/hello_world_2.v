// A variation on hello_world_1.v. Uses the concatenation operator to 
// map buttons onto more leds. This program is compatible with any march
// file that exposes at least as many leds as buttons.

assign led.val = {pad.val, pad.val};
