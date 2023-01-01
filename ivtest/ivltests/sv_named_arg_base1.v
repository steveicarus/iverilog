// Check that binding task arguments by name is supported.

module test;

  class B;
    integer val;

    function new(integer a, integer b);
      val = a + b * 10;
    endfunction
  endclass

  class C extends B(.b(2), .a(1));
  endclass

  initial begin
    C c;
    c = new;
    if (c.val == 21) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
