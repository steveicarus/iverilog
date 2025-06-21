module top;
  reg  passed;
  real inr;
  wire out_buf, out_not, out_and, out_nand, out_or, out_nor, out_xor, out_xnor;

  buf(out_buf, inr);
  not(out_not, inr);
  and(out_and, inr, 1'b1);
  nand(out_nand, inr, 1'b1);
  or(out_or, inr, 1'b0);
  nor(out_nor, inr, 1'b0);
  xor(out_xor, inr, 1'b1);
  xnor(out_xnor, inr, 1'b1);

  always @(out_not) begin
    int inr_int;
    inr_int = inr;
    if (out_not !== !inr_int[0]) begin
      $display("not of %f not equal to %b", inr, !inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_buf) begin
    int inr_int;
    inr_int = inr;
    if (out_buf !== inr_int[0]) begin
      $display("buf of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_and) begin
    int inr_int;
    inr_int = inr;
    if (out_and !== inr_int[0]) begin
      $display("and of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_nand) begin
    int inr_int;
    inr_int = inr;
    if (out_nand !== !inr_int[0]) begin
      $display("nand of %f not equal to %b", inr, !inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_or) begin
    int inr_int;
    inr_int = inr;
    if (out_or !== inr_int[0]) begin
      $display("or of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_nor) begin
    int inr_int;
    inr_int = inr;
    if (out_nor !== !inr_int[0]) begin
      $display("nor of %f not equal to %b", inr, !inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_xor) begin
    int inr_int;
    inr_int = inr;
    if (out_xor !== !inr_int[0]) begin
      $display("xor of %f not equal to %b", inr, !inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_xnor) begin
    int inr_int;
    inr_int = inr;
    if (out_xnor !== inr_int[0]) begin
      $display("xnor of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  initial begin
    $display("bf nt an na or no xo xn in");
    $monitor(out_buf,,,out_not,,,out_and,,,out_nand,,,out_or,,,out_nor,,,out_xor,,,out_xnor,,,inr);
    #1;
    if (passed === 1'bx) passed = 1'b1;

    inr = 1.0;
    #1;
    inr = 0.0;
    #1
    inr = 3.0;
    #1;
    inr = 2.0;
    #1;
    inr = 4.0;
    #1;
    inr = 2.5;
    #1;
    inr = 2.49;
    #1;
    inr = -1.0;
    #1;
    inr = 1.0/0.0;
    #1;

    if (passed) $display("PASSED");
  end

endmodule
