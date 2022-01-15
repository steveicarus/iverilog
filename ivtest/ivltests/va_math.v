module top;
  parameter pabsi = $abs(-1);
  parameter pabsr = $abs(-1.5);

  parameter pmini = $min(1, 2);
  parameter pminra = $min(1, 2.5);
  parameter pminrb = $min(1.5, 2);
  parameter pminrc = $min(1.5, 2.5);
  parameter pminia = $min(1, 3'b010);
  parameter pminib = $min(1, 3'b110);
  parameter pmaxi = $max(1, 2);
  parameter pmaxr = $max(1.5, 2.5);

  reg pass = 1'b1;

  real resr, rin;
  integer resi, iin;

  initial begin

    /* Test the elab_pexpr implementation. */

    if (pabsi != 1) begin
      $display("Failed with param. $abs (int), expected 1, got %d", pabsi);
      pass = 1'b0;
    end
    if (pabsr != 1.5) begin
      $display("Failed with param. $abs (real), expected 1.5, got %f", pabsr);
      pass = 1'b0;
    end

    if (pmini != 1) begin
      $display("Failed with param. $min (int), expected 1, got %d", pmini);
      pass = 1'b0;
    end
    if (pminra != 1.0) begin
      $display("Failed with param. $min a(real), expected 1.0, got %f", pminra);
      pass = 1'b0;
    end
    if (pminrb != 1.5) begin
      $display("Failed with param. $min b(real), expected 1.5, got %f", pminrb);
      pass = 1'b0;
    end
    if (pminrc != 1.5) begin
      $display("Failed with param. $min c(real), expected 1.5, got %f", pminrc);
      pass = 1'b0;
    end
    if (pminia != 1.0) begin
      $display("Failed with param. $min a(int), expected 1, got %d", pminia);
      pass = 1'b0;
    end
    if (pminib != 1) begin
      $display("Failed with param. $min b(real), expected 1, got %d", pminib);
      pass = 1'b0;
    end
    if (pmaxi != 2) begin
      $display("Failed with param. $max (int), expected 2, got %d", pmaxi);
      pass = 1'b0;
    end
    if (pmaxr != 2.5) begin
      $display("Failed with param. $max (real), expected 2.5, got %f", pmaxr);
      pass = 1'b0;
    end

    /* Test the eval_tree implementation. */

    resi = $abs(-1);
    if (resi != 1) begin
      $display("Failed with $abs (int), expected 1, got %d", resi);
      pass = 1'b0;
    end
    resr = $abs(-1.5);
    if (resr != 1.5) begin
      $display("Failed with $abs (real), expected 1.5, got %f", resr);
      pass = 1'b0;
    end

    resi = $min(1, 2);
    if (resi != 1) begin
      $display("Failed with $min (int), expected 1, got %d", resi);
      pass = 1'b0;
    end
    resr = $min(1.5, 2.5);
    if (resr != 1.5) begin
      $display("Failed with $min (real), expected 1.5, got %f", resr);
      pass = 1'b0;
    end

    resi = $max(1, 2);
    if (resi != 2) begin
      $display("Failed with $max (int), expected 2, got %d", resi);
      pass = 1'b0;
    end
    resr = $max(1.5, 2.5);
    if (resr != 2.5) begin
      $display("Failed with $max (real), expected 2.5, got %f", resr);
      pass = 1'b0;
    end

    // Run time generated.
    iin = -1;
    resi = $abs(iin);
    if (resi != 1) begin
      $display("Failed run time with $abs (int), expected 1 got %d", resi);
      pass = 1'b0;
    end
    rin = -1.5;
    resr = $abs(rin);
    if (resr != 1.5) begin
      $display("Failed run time with $abs (real), expected 1.5 got %f", resr);
      pass = 1'b0;
    end

    iin = 1;
    resi = $min(iin, 2);
    if (resi != 1) begin
      $display("Failed run time with $min (int), expected 1 got %d", resi);
      pass = 1'b0;
    end
    rin = 1.5;
    resr = $min(rin, 2.5);
    if (resr != 1.5) begin
      $display("Failed run time with $min (real), expected 1.5 got %f", resr);
      pass = 1'b0;
    end

    iin = 1;
    resi = $max(iin, 2);
    if (resi != 2) begin
      $display("Failed run time with $max (int), expected 2 got %d", resi);
      pass = 1'b0;
    end
    rin = 1.5;
    resr = $max(rin, 2.5);
    if (resr != 2.5) begin
      $display("Failed run time with $max (real), expected 2.5 got %f", resr);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
