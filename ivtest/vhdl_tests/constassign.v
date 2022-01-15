// A very simple test to check continuous assignment
// of a constant
module main();
  wire p;

  assign p = 1;

  initial begin
    #1;
    if (p == 1)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule // main
