module top;
  localparam [6:0] with_range = 63;
  localparam no_range = 1;

  initial if (with_range != 63 || no_range != 1) $display("FAILED");
  else $display("PASSED");
endmodule
