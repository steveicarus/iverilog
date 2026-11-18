`timescale 1ps/1ps

primitive delay_mux_udp (q, a, b, s);
  output q;
  input a, b, s;
  table
    0 ? 0 : 0;
    1 ? 0 : 1;
    ? 0 1 : 0;
    ? 1 1 : 1;
  endtable
endprimitive

module delay_mux (output q, input a, b, s);
  delay_mux_udp logic_gate(q, a, b, s);
  specify
    (a => q) = (3,5,9,9,9,9,9,9,9,9,9,9);
    (b => q) = (3,5,9,9,9,9,9,9,9,9,9,9);
    (s => q) = (3,5,9,9,9,9,9,9,9,9,9,9);
  endspecify

  initial begin
    #90 force q = 1'b0;
    #30 release q;
  end
endmodule

module no_path (output q, input a);
  not logic_gate(q, a);
endmodule

module packed_path (output [1:0] q, input a);
  not invert(q[0], a);
  buf buffer(q[1], a);
  specify (a *> q) = (3,5); endspecify
endmodule

module resolved_path (output q, input a, b);
  buf left_gate(q, a);
  buf right_gate(q, b);
  specify (a => q) = (3,5); endspecify
endmodule

module test;
  reg a, b, s;
  wire q, ordinary_q, resolved_q;
  wire [1:0] packed_q;
  delay_mux dut(q, a, b, s);
  no_path ordinary(ordinary_q, a);
  packed_path packed_owner(packed_q, a);
  resolved_path resolved_owner(resolved_q, a, b);

  initial begin
    a = 0; b = 1; s = 0;
    #100 s = 1;    // Delay input becomes 1; delivery at 103 while forced.
    #100 b = 0;    // Delay input becomes 0; delivery at 205.
    #100 b = 1'bx; // Delay input becomes X; delivery at 309.
    #100 b = 1;    // Delay input becomes 1; delivery at 409.
    #100;
  end
endmodule
