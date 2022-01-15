module top;
  reg ctl, in0, in1;
  wire out3, out4;

  // A UDP can only take two delay values.
  mux2 #(10, 20, 30) q1(out3, ctl, in0, in1);
  mux2 #(10, 20, 30, 40) q2(out4, ctl, in0, in1);

  initial $display("FAILED");
endmodule

primitive mux2 (out, ctl, in0, in1);
  output out;
  input ctl, in0, in1;

  table
    0 0 ? : 0;
    0 1 ? : 1;
    1 ? 0 : 0;
    1 ? 1 : 1;
    x 0 0 : 0;
    x 1 1 : 1;
  endtable
endprimitive
