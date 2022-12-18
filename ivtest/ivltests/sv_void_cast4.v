// Check that void casts on SystemFunctions is supported

module test;

  initial begin
    void'($clog2(10));

    $display("PASSED");
  end

endmodule
