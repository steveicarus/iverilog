// Check that binding task arguments by name is supported.

module test;

  class C;
    integer val;

    function new(integer a, integer b);
      val = a + b * 10;
    endfunction
  endclass

  initial begin
    C c;
    c = new(.b(2), .a(1));
    if (c.val == 21) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
