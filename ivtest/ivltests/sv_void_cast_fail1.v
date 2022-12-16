// Check that void casting a void function results in an error

module test;

  function void f(int x);
  endfunction

  initial begin
    void'(f(10));
  end

endmodule
