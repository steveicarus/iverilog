module top;
  reg pass;
  real rval;
  reg [7:0] res;

  initial begin
    pass = 1'b1;
    res = 6.0;
    if (res !== 8'd6) begin
      $display("Failed blocking assignment, expeted 6, got %d", res);
      pass = 1'b0;
    end

    // The compiler is generating bad code for a NB-assign with a real r-value.
    res <= 7.0;
    #1 if (res !== 8'd7) begin
      $display("Failed nonblocking assignment, expeted 7, got %d", res);
      pass = 1'b0;
    end
    rval = 8.0;
    res <= rval;
    #1 if (res !== 8'd8) begin
      $display("Failed nonblocking assignment, expeted 8, got %d", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
