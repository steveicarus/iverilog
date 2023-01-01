// Check that binding task arguments by name is supported and that a mix of
// positional and named arguments is supported.

module test;

  function integer f(integer a, integer b, integer c);
    return a + b * 10 + c * 100;
  endfunction

  initial begin
    integer x;
    x = f(1, .c(3), .b(2));
    if (x == 321) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
