// Check that binding task arguments by name is supported and that an empty
// value can be bound to the name, in which case the default argument value
// should be used.

module test;

  class C;
    integer val;

    function new(integer a, integer b = 2);
      val = a + b * 10;
    endfunction
  endclass

  initial begin
    C c;
    c = new(.a(1), .b());
    if (c.val == 21) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end
endmodule
