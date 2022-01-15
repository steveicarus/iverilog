module top;
  integer in;

  initial begin
    in = 2;
    if ($clog2(in) != 1) $display("FAILED");
    else $display("PASSED");
  end
endmodule
