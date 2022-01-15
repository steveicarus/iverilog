`timescale 1ns/100ps
module top;
  reg pass;
  reg in;
  wire out_bit;
  wire [3:0] out_vec, out_arr;
  wire real r_bit, r_vec;
  wire real r_arr[1:0];

  initial begin
    pass = 1'b1;
    in <= 1'b0;
    #1 in = 1'b1;
    #0.5 in = 1'b0;
    #0.5 in = 1'b1;
    #0.5 in = 1'b0;
    #0.5 in = 1'b1;
    #0.5 in = 1'b0;
    #0.5 in = 1'b1;
    #1 if (pass) $display("PASSED");
  end

  real_to_xx u1(out_bit, in);
  always @(out_bit) if (out_bit !== ($stime % 2)) begin
    $display("Failed real_to_xx, got %b, expected %1b", out_bit, $stime%2);
    pass = 1'b0;
  end

  real_to_xx u2(out_vec, in);
  always @(out_vec) if (out_vec !== $stime) begin
    $display("Failed real_to_xx(vec), got %b, expected %2b", out_vec, $stime);
    pass = 1'b0;
  end

  real_to_xx u3[1:0](out_arr, in);
  always @(out_arr) #0.1 if ((out_arr[1:0] !== ($stime % 4)) &&
                             (out_arr[3:2] !== ($stime % 4))) begin
    $display("Failed real_to_xx[1:0], got %b, expected %2b%2b", out_arr,
             $stime%4, $stime%4);
    pass = 1'b0;
  end

  bit_to_real u4(r_bit, in);
  always @(r_bit) if (r_bit != ($stime % 2)) begin
    $display("Failed bit_to_real, got %f, expected %1b", r_bit, $stime%2);
    pass = 1'b0;
  end

  vec_to_real u5(r_vec, in);
  always @(r_vec) if (r_vec != $stime) begin
    $display("Failed vec_to_real, got %f, expected %1b", r_vec, $stime);
    pass = 1'b0;
  end

endmodule

// Check a real value going to a various things.
module real_to_xx (output wire real out, input wire in);
  real rval;
  assign out = rval;
  always @(posedge in) rval = rval + 1;
endmodule

module bit_to_real (output wire out, input wire in);
  reg rval = 0;
  assign out = rval;
  always @(posedge in) rval = rval + 1;
endmodule

module vec_to_real (output wire [3:0] out, input wire in);
  reg [3:0] rval = 0;
  assign out = rval;
  always @(posedge in) rval = rval + 1;
endmodule
