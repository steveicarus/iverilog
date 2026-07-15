// Check that an ANSI-style UDP reg output can have an initializer.

primitive udp_ansi_initial_reg (output reg o = 1'b0, input i);
  table
    0 : ? : 0;
    1 : ? : 1;
  endtable
endprimitive

module test;
  reg i;
  wire o;

  udp_ansi_initial_reg i_udp(o, i);

  initial begin
    i = 1'b0;
    #1;
    $display("PASSED");
  end
endmodule
