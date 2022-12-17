// Check that an error is reported when a void function is used in an expression

module test;

  function void f;
  endfunction

  initial begin
    int x;
    x = f() + 1; // This should fail, void function can not be used in expression
    $display("FAILED");
  end

endmodule
