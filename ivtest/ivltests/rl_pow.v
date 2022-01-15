`timescale 1us/100ns

module top;
  reg pass = 1'b1;

  real ra, rb, rpow;

  initial begin
    ra = 1.0; rb = 2.0;
    rpow = ra ** rb;
    if (rpow != 1.0) begin
      pass = 1'b0;
      $display("Real: power value not correct, expected 1.0 got %g.", rpow);
    end

    ra = 2.0;
    rpow = ra ** rb;
    if (rpow != 4.0) begin
      pass = 1'b0;
      $display("Real: power value not correct, expected 4.0 got %g.", rpow);
    end

    ra = 0.0;
    rpow = ra ** rb;
    if (rpow != 0.0) begin
      pass = 1'b0;
      $display("Real: power value not correct, expected 0.0 got %g.", rpow);
    end

    ra = 10.0;
    rpow = ra ** rb;
    if (rpow != 100.0) begin
      pass = 1'b0;
      $display("Real: power value not correct, expected 100.0 got %g.", rpow);
    end

    ra = 0.0; rb = -1.0;
    rpow = ra ** rb;
    $display("0.0 ** -1.0 = %g", rpow);

    ra = -1.0; rb = 2.5;
    rpow = ra ** rb;
    $display("-1.0 ** 2.5 = %g", rpow);

    if (pass) $display("PASSED");
  end
endmodule
