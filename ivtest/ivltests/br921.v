module top;

  initial begin
    // In SystemVerilog a function is not required to return a value or
    // to take an argument.
    test_fcn();
    $display("PASSED");
  end

  function bit test_fcn();
    test_fcn = 1'b1;
  endfunction

endmodule
