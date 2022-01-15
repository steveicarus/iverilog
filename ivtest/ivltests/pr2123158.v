module top;
  reg pass = 1'b1;
  wire real rval;
  real in;

  assign rval = in + 2;

  initial begin
    // $monitor(rval,, in);
    in = 0;
    #1 in = 1;
    #1 in = 2;
    #1 if (pass) $display("PASSED");
  end

  always @(rval) begin
    if (rval != in + 2.0) begin
      $display("FAILED: expected %f, got %f", in + 2.0, rval);
      pass = 1'b0;
    end
  end
endmodule
