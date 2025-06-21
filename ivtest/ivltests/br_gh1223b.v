module top;
  reg  passed;
  real inr, ctrl;
  wire out_bf0, out_bf1, out_nt0, out_nt1;

  bufif0(out_bf0, inr, ctrl);
  bufif1(out_bf1, inr, 1'b1);
  notif0(out_nt0, inr, ctrl);
  notif1(out_nt1, inr, 1'b1);

  always @(out_bf0) begin
    int inr_int;
    inr_int = inr;
    if (out_bf0 !== inr_int[0]) begin
      $display("bufif0 of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_bf1) begin
    int inr_int;
    inr_int = inr;
    if (out_bf1 !== inr_int[0]) begin
      $display("bufif1 of %f not equal to %b", inr, inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_nt0) begin
    int inr_int;
    inr_int = inr;
    if (out_nt0 !== !inr_int[0]) begin
      $display("bufif0 of %f not equal to %b", inr, !inr_int[0]);
      passed = 1'b0;
    end
  end

  always @(out_nt1) begin
    int inr_int;
    inr_int = inr;
    if (out_nt1 !== !inr_int[0]) begin
      $display("bufif1 of %f not equal to %b", inr, !inr_int[0]);
      passed = 1'b0;
    end
  end

  initial begin
    $display("b0 b1 n0 n1 in");
    $monitor(out_bf0,,,out_bf1,,,out_nt0,,,out_nt1,,,inr);
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
