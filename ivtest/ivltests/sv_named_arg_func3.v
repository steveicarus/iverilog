// Check that binding task arguments by name is supported and that an empty
// value can be bound to the name, in which case the default argument value
// should be used.

module test;

  function integer f(integer a, integer b = 2);
    return a + b * 10;
  endfunction

  initial begin
    integer x;
    x = f(.a(1), .b());
    if (x == 21) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
