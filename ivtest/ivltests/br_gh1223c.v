module top;
  reg  passed;
  real inr, ctrl;
  wire out_nm, out_rnm, out_pm, out_rpm;
  wire out_cm_n, out_cm_p, out_rcm_n, out_rcm_p;

  pmos(out_pm, inr, ctrl);
  rpmos(out_rpm, inr, ctrl);
  nmos(out_nm, inr, 1'b1);
  rnmos(out_rnm, inr, 1'b1);

  cmos(out_cm_n, inr, 1'b1, 1'b1);
  cmos(out_cm_p, inr, ctrl, ctrl);
  cmos(out_rcm_n, inr, 1'b1, 1'b1);
  cmos(out_rcm_p, inr, ctrl, ctrl);

  always @(out_pm) begin
    int inr_int;
    inr_int = inr;
    if (out_pm !== inr_int[0]) begin
      $display("pmos of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_rpm) begin
    int inr_int;
    inr_int = inr;
    if (out_rpm !== inr_int[0]) begin
      $display("rpmos of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_nm) begin
    int inr_int;
    inr_int = inr;
    if (out_nm !== inr_int[0]) begin
      $display("nmos of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_rnm) begin
    int inr_int;
    inr_int = inr;
    if (out_rnm !== inr_int[0]) begin
      $display("rnmos of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_cm_n) begin
    int inr_int;
    inr_int = inr;
    if (out_cm_n !== inr_int[0]) begin
      $display("cmos(N) of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_cm_p) begin
    int inr_int;
    inr_int = inr;
    if (out_cm_p !== inr_int[0]) begin
      $display("cmos(P) of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_rcm_n) begin
    int inr_int;
    inr_int = inr;
    if (out_rcm_n !== inr_int[0]) begin
      $display("rcmos(N) of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_rcm_p) begin
    int inr_int;
    inr_int = inr;
    if (out_rcm_p !== inr_int[0]) begin
      $display("rcmos(P) of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  initial begin
    $display("n  rn p  rp cn rcn cp rcp in");
    $monitor(out_nm,,,out_rnm,,,out_pm,,,out_rpm,,,out_cm_n,,,out_rcm_n,,,,out_cm_p,,,out_rcm_p,,,,inr);
    #1;
    if (passed === 1'bx) passed = 1'b1;

    ctrl = 2.49;
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
