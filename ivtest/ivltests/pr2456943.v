module top;
  wire real test;

  initial begin
    if (test != 0.0) $display("FAILED");
    else $display("PASSED");
  end
endmodule
