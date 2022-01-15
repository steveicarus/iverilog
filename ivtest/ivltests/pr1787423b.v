module top;
  reg pass = 1'b1;
  reg in;
  wire bf1, bf2, nt1, nt2, pd1, pd2, pu1, pu2;

  initial begin
    // $monitor(bf1, bf2,, nt1, nt2,, pd1, pd2,, pu1, pu2,, in);
    if (bf1 !== 1'bx && bf2 !== 1'bx) begin
      $display("Buffer failed, expected 2'bxx, got %b%b", bf1, bf2);
      pass = 1'b0;
    end
    if (nt1 !== 1'bx && nt2 !== 1'bx) begin
      $display("Inverter (not) failed, expected 2'bxx, got %b%b", nt1, nt2);
      pass = 1'b0;
    end
    if (pd1 !== 1'b0 && pd2 !== 1'b0) begin
      $display("Pull down failed, expected 2'b00, got %b%b", pd1, pd2);
      pass = 1'b0;
    end
    if (pu1 !== 1'b1 && pu2 !== 1'b1) begin
      $display("Pull up failed, expected 2'b11, got %b%b", pu1, pu2);
      pass = 1'b0;
    end

    in = 1'b0;
    #1;
    if (bf1 !== 1'b0 && bf2 !== 1'b0) begin
      $display("Buffer failed, expected 2'b00, got %b%b", bf1, bf2);
      pass = 1'b0;
    end
    if (nt1 !== 1'b1 && nt2 !== 1'b1) begin
      $display("Inverter (not) failed, expected 2'b11, got %b%b", nt1, nt2);
      pass = 1'b0;
    end
    if (pd1 !== 1'b0 && pd2 !== 1'b0) begin
      $display("Pull down failed, expected 2'b00, got %b%b", pd1, pd2);
      pass = 1'b0;
    end
    if (pu1 !== 1'b1 && pu2 !== 1'b1) begin
      $display("Pull up failed, expected 2'b11, got %b%b", pu1, pu2);
      pass = 1'b0;
    end

    in = 1'b1;
    #1;
    if (bf1 !== 1'b1 && bf2 !== 1'b1) begin
      $display("Buffer failed, expected 2'b11, got %b%b", bf1, bf2);
      pass = 1'b0;
    end
    if (nt1 !== 1'b0 && nt2 !== 1'b0) begin
      $display("Inverter (not) failed, expected 2'b00, got %b%b", nt1, nt2);
      pass = 1'b0;
    end
    if (pd1 !== 1'b0 && pd2 !== 1'b0) begin
      $display("Pull down failed, expected 2'b00, got %b%b", pd1, pd2);
      pass = 1'b0;
    end
    if (pu1 !== 1'b1 && pu2 !== 1'b1) begin
      $display("Pull up failed, expected 2'b11, got %b%b", pu1, pu2);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end

  buf (bf1, bf2, in);
  not (nt1, nt2, in);
  pulldown (pd1, pd2);
  pullup (pu1, pu2);
endmodule
