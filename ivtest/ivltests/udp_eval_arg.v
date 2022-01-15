module top;
  reg pass;

  tri ctl;
  reg in0, in1;
  wire out;
  integer a;

  // Code to force two drivers.
  pullup (ctl);
  assign ctl = a ? 1'bz : 1'b0;

  mux2 q1(out, ctl, in0, in1);

  initial begin
    pass = 1'b1;
    a = 0;
    in0 = 1'b0;
    in1 = 1'b1;
    #1;
    if (out !== 1'b0) begin
      $display("Failed UDP with ctl 0, expected 1'b0, got %b", out);
      pass = 1'b0;
    end
    a = 1;
    #1;
    if (out !== 1'b1) begin
      $display("Failed UDP with ctl 1, expected 1'b1, got %b", out);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
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
