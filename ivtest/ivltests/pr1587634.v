//
// Test the specify block (pr1587634)
//

`timescale 1 ns / 1 ps

/*
module top();
  reg in;
  initial begin
    in = 0;
    #10 in = 1;
  end

  inv1 g1 (out, in);
endmodule
*/

module inv1 (z, a);
  output z;
  input  a;

  not g1(z, a);

  specify
    specparam tpd_a_z_lh = 0.400;
    specparam tpd_a_z_hl = 0.300:0.400:0.500;

    (a -=> z) = (tpd_a_z_lh, tpd_a_z_hl);

  endspecify
endmodule
