`timescale 1ns/10ps

module top;
  reg pass;
  real result;

  initial begin
    pass = 1'b1;

    result = $abstime;
    if (result != 0.0) begin
      $display("FAILED at time 0, expected 0.0, got %g", result);
      pass = 1'b0;
    end

    #10;
    result = $abstime;
    if ($abs(result-10e-9) > result*1e-9) begin
      $display("FAILED at time 10ns, expected 1e-8, got %g", result);
      pass = 1'b0;
    end

    #999990;
    result = $abstime;
    if ($abs(result-0.001) > result*1e-9) begin
      $display("FAILED at time 1ms, expected 0.001, got %g", result);
      pass = 1'b0;
    end

`ifdef __ICARUS_UNSIZED__
    #9999000000;
`else
    #9999000000.0;
`endif
    result = $abstime;
    if ($abs(result-10.0) > result*1e-9) begin
      $display("FAILED at time 10s, expected 10.0, got %g", result);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
