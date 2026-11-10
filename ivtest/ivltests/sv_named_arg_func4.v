// Check that an empty positional argument can precede a named argument.

module test;

  function integer f(integer a = 1, integer b = 2);
    f = 10*a + b;
  endfunction

  initial begin
    if (f(, .b(9)) === 19) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
