`timescale 1ns/100ps

module top;
  reg pass = 1'b1;
  integer idelay = 2;
  real rdelay = 2.0;
  real rin = 1.0;
  reg ctl = 1'b1;
  wire outr, outi, muxr, muxi;
  wire real rout;
  reg in = 1'b1;

  assign #(idelay) outi = in;
  assign #(rdelay) outr = ~in;
  assign #(rdelay) rout = rin;
  assign #(idelay) muxi = ctl ? in : 1'b0;
  assign #(rdelay) muxr = ctl ? ~in : 1'b1;

  initial begin
    // Wait for everything to settle including the delay value!
    #2.1;
    if (outi !== 1'b1 || outr !== 1'b0 || rout != 1.0) begin
      $display("FAILED: initial value, expected 1'b1/1'b0/1.0, got %b/%b/%f",
               outi, outr, rout);
      pass = 1'b0;
    end
    if (muxi !== 1'b1 || muxr !== 1'b0) begin
      $display("FAILED: initial value (mux), expected 1'b1/1'b0, got %b/%b",
               muxi, muxr);
      pass = 1'b0;
    end

    in = 1'b0;
    rin = 2.0;
    #1.9;
    if (outi !== 1'b1 || outr !== 1'b0 || rout != 1.0) begin
      $display("FAILED: mid value, expected 1'b1/1'b0/1.0, got %b/%b/%f",
               outi, outr, rout);
      pass = 1'b0;
    end
    if (muxi !== 1'b1 || muxr !== 1'b0) begin
      $display("FAILED: mid value (mux), expected 1'b1/1'b0, got %b/%b",
               muxi, muxr);
      pass = 1'b0;
    end
    #0.2;
    if (outi !== 1'b0 || outr !== 1'b1 || rout != 2.0) begin
      $display("FAILED: final value, expected 1'b0/1'b1/2.0, got %b/%b/%f",
               outi, outr, rout);
      pass = 1'b0;
    end
    if (muxi !== 1'b0 || muxr !== 1'b1) begin
      $display("FAILED: final value (mux), expected 1'b0/1'b1, got %b/%b",
               muxi, muxr);
      pass = 1'b0;
    end

    idelay = 3;
    in = 1'b1;
    #2.9;
    if (outi !== 1'b0) begin
      $display("FAILED: initial change, expected 1'b0, got %b", outi);
      pass = 1'b0;
    end
    #0.2;
    if (outi !== 1'b1) begin
      $display("FAILED: initial change, expected 1'b1, got %b", outi);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
