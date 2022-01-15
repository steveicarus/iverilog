`timescale 1us/100ns

module top;
  reg pass = 1'b1;

  real ra = 1.0, rb = 2.0;
  wire real rpow;

  /* Real Power. */
  assign #1 rpow = ra ** rb;

  initial begin
    #0.9;
    if (rpow == 1.0) begin
      pass = 1'b0;
      $display("Real: power value not delayed.");
    end

    #0.1;
    #0;
    if (rpow != 1.0) begin
      pass = 1'b0;
      $display("Real: power value not correct, expected 1.0 got %g.", rpow);
    end

    #1 ra = 2.0;
    #2;
    if (rpow != 4.0) begin
      pass = 1'b0;
      $display("Real: power value not correct, expected 4.0 got %g.", rpow);
    end

    #1 ra = 0.0;
    #2;
    if (rpow != 0.0) begin
      pass = 1'b0;
      $display("Real: power value not correct, expected 0.0 got %g.", rpow);
    end

    #1 ra = 10.0;
    #2;
    if (rpow != 100.0) begin
      pass = 1'b0;
      $display("Real: power value not correct, expected 100.0 got %g.", rpow);
    end

    #1 ra = 0.0; rb = -1.0;
    #2;
    $display("0.0 ** -1.0 = %g", rpow);

    #1 ra = -1.0; rb = 2.5;
    #2;
    $display("-1.0 ** 2.5 = %g", rpow);

    if (pass) $display("PASSED");
  end
endmodule
