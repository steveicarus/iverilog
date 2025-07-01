module top;
  real  rout_ca1, rout_ca2, rout_valid, rout_gt, rout_udp;
  logic lout_ca1, lout_ca2, lout_valid1, lout_valid2, lout_gt, lout_udp;
  reg in;

  assign (weak1, weak0) {rout_ca1, rout_ca2} = in; // Non-default strength so invalid
  assign (weak1, weak0) {lout_ca1, lout_ca2} = in; // Non-default strength so invalid

  assign (strong1, strong0) rout_valid = in; // Ok, real cannot be in a concatenation
  assign (strong1, strong0) {lout_valid1, lout_valid2} = in; // Ok, default strength

  and (rout_gt, in, in); // Gates must drive a net
  and (lout_gt, in, in); // Gates must drive a net

  // When strength is added it should only be for the default strength!
  udp_inv (rout_udp, in); // A UDP is like a module and can drive a variable
  udp_inv (lout_udp, in); // A UDP is like a module and can drive a variable

  initial $display("FAILED: There should be compile errors!");
endmodule

primitive udp_inv (output y, input a);
  table
    0 : 1;
    1 : 0;
  endtable
endprimitive
