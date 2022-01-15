`timescale 1s/1s

module test(outp, outm, outl, in);
  output outp, outm, outl;
  input in;

  // Check a primitive.
  assign #1 outp = ~in;

  // Check a multiplexer.
  assign #1 outm = in ? in : 1'b0;

  // Check a LPM.
  assign #1 outl = in === 1'b1;
endmodule

// This is not exactly the same as the original code, but it is effectively
// the same and should test the same things that were failing.
`timescale 1ns/100ps

module top;
  reg in, passed;
  wire outp, outm, outl;

  test dut(outp, outm, outl, in);

  initial begin
    passed = 1'b1;

    #1100000000;
    if (outp !== 1'bx) begin
      $display("Failed initial prim. check, expected 1'bx, got %b.", outp);
      passed = 1'b0;
    end
    if (outm !== 1'bx) begin
      $display("Failed initial mux. check, expected 1'bx, got %b.", outm);
      passed = 1'b0;
    end
    if (outl !== 1'b0) begin
      $display("Failed initial LPM check, expected 1'b0, got %b.", outl);
      passed = 1'b0;
    end

    in = 0;
    #1100000000;
    if (outp !== 1'b1) begin
      $display("Failed in=0 prim. check, expected 1'b1, got %b.", outp);
      passed = 1'b0;
    end
    if (outm !== 1'b0) begin
      $display("Failed in=0 mux. check, expected 1'b0, got %b.", outm);
      passed = 1'b0;
    end
    if (outl !== 1'b0) begin
      $display("Failed in=0 LPM check, expected 1'b0, got %b.", outl);
      passed = 1'b0;
    end

    in = 1;
    #1100000000;
    if (outp !== 1'b0) begin
      $display("Failed in=1 prim. check, expected 1'b0, got %b.", outp);
      passed = 1'b0;
    end
    if (outm !== 1'b1) begin
      $display("Failed in=1 mux. check, expected 1'b1, got %b.", outm);
      passed = 1'b0;
    end
    if (outl !== 1'b1) begin
      $display("Failed in=1 LPM check, expected 1'b1, got %b.", outl);
      passed = 1'b0;
    end

    if (passed) $display("PASSED");

  end
endmodule
