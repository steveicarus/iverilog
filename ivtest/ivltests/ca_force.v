module top;
  reg pass;
  reg in;
  wire ca;

  assign ca = in;

  initial begin
    pass = 1'b1;
    if (ca !== 1'bx || in !== 1'bx) begin
      $display("Failed T0 check, in %b, ca %b", in, ca);
      pass = 1'b0;
    end

    in = 1'b0;
    #1;
    if (ca !== 1'b0 || in !== 1'b0) begin
      $display("Failed 0 check, in %b, ca %b", in, ca);
      pass = 1'b0;
    end

    in = 1'b1;
    #1;
    if (ca !== 1'b1 || in !== 1'b1) begin
      $display("Failed 1 check, in %b, ca %b", in, ca);
      pass = 1'b0;
    end

    force ca = 1'b0;
    #1;
    if (ca !== 1'b0 || in !== 1'b1) begin
      $display("Failed force 0 check, in %b, ca %b", in, ca);
      pass = 1'b0;
    end

    in = 1'bx;
    #1;
    if (ca !== 1'b0 || in !== 1'bx) begin
      $display("Failed change a check, in %b, ca %b", in, ca);
      pass = 1'b0;
    end

    force ca = 1'b1;
    #1;
    if (ca !== 1'b1 || in !== 1'bx) begin
      $display("Failed force 1 check, in %b, ca %b", in, ca);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
