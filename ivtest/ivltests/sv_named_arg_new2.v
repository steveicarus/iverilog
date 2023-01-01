// Check that binding task arguments by name is supported and that a mix of
// positional and named arguments is supported.

module test;

  class C;
    integer val;

    function new(integer a, integer b, integer c);
      val = a + b * 10 + c * 100;
    endfunction
  endclass

  initial begin
    C c;
    c = new(1, .c(3), .b(2));
    if (c.val == 321) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
