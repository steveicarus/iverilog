// Check that binding task arguments by name is supported.

module test;

  function integer f(integer a, integer b);
   return a + b * 10;
  endfunction

  initial begin
    integer x;
    x = f(.b(2), .a(1));
    if (x == 21) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
   end

endmodule
