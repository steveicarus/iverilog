`timescale 1ns/100ps
module top;
  reg in;
  wire [3:0] vec;
  wire [4:0] bvec;
  wire real r_vec, r_arr, r_io;

  initial in <= 1'b0;

  // You cannot go to multiple real values (have multiple instances).
  vec_to_real u1[1:0](r_vec, in);

  // A real port cannot be used in an arrayed instance.
  arr_real u2a[1:0](bvec, in);
  arr_real u2b[1:0](r_arr, in);

  // You cannot connect a real to an inout port.
  io_vec_to_real u3(r_io, in);

  // You cannot have a inout port declared real.
  io_real_to_vec u4(vec, in);

endmodule

module vec_to_real (output wire [3:0] out, input wire in);
  reg [3:0] rval = 0;
  assign out = rval;
  always @(posedge in) rval = rval + 1;
endmodule

module arr_real(output wire real out, input wire in);
  real rval;
  assign out = rval;
  always @(posedge in) rval = rval + 1;
endmodule

module io_vec_to_real(inout wire [3:0] out, input wire in);
  reg [3:0] rval = 0;
  assign out = rval;
  always @(posedge in) rval = rval + 1;
endmodule

module io_real_to_vec(inout wire real out, input wire in);
  real rval;
  assign out = rval;
  always @(posedge in) rval = rval + 1;
endmodule
