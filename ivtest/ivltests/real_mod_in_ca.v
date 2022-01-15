`timescale 1us/100ns

module top;
  reg pass = 1'b1;

  real ra = 1.0, rb = 2.0;
  wire real rmod;

  /* Real Power. */
  assign #1 rmod = ra % rb;

  initial begin
    #0.9;
    if (rmod == 1.0) begin
      pass = 1'b0;
      $display("Real: modulus value not delayed.");
    end

    #0.1;
    #0;
    if (rmod != 1.0) begin
      pass = 1'b0;
      $display("Real: modulus value not correct, expected 1.0 got %g.", rmod);
    end

    #1 ra = 2.0;
    #2;
    if (rmod != 0.0) begin
      pass = 1'b0;
      $display("Real: modulus value not correct, expected 0.0 got %g.", rmod);
    end

    #1 rb = 4.0;
    #2;
    if (rmod != 2.0) begin
      pass = 1'b0;
      $display("Real: modulus value not correct, expected 2.0 got %g.", rmod);
    end

    if (pass) $display("PASSED");
  end
endmodule
