module top;
  reg pass;
  real in;
  wreal out1, out2, outa;
  wreal ca1 = 2.25;
  wreal ca2;

  assign ca2 = 4.5;

  sub1 dut1(out1, in);
  sub2 dut2(out2, in);
  suba duta(outa, in);

  initial begin
    pass = 1'b1;
    in = 1.0;
    #1;
    if (out1 != 1.0) begin
      $display("FAILED: expected out1 to be 1.0, got %f", out1);
      pass = 1'b0;
    end
    if (out2 != 1.0) begin
      $display("FAILED: expected out2 to be 1.0, got %f", out2);
      pass = 1'b0;
    end
    if (outa != 1.0) begin
      $display("FAILED: expected outa to be 1.0, got %f", outa);
      pass = 1'b0;
    end
    if (ca1 != 2.25) begin
      $display("FAILED: expected ca1 to be 2.25, got %f", ca1);
      pass = 1'b0;
    end
    if (ca2 != 4.5) begin
      $display("FAILED: expected ca1 to be 4.5, got %f", ca2);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule

module sub1(out, in);
  output out;
  input in;
  wreal out, in;
  assign out = in;
endmodule

module sub2(out, in);
  output wreal out;
  input wreal in;
  assign out = in;
endmodule

module suba(output wreal out, input wreal in);
  assign out = in;
endmodule
