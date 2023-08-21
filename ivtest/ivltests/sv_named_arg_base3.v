// Check that binding task arguments by name is supported and that an empty
// value can be bound to the name, in which case the default argument value
// should be used.

module test;

  class B;
    integer val;

    function new(integer a, integer b = 2);
      val = a + b * 10;
    endfunction
  endclass

  class C extends B(.a(1), .b());
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
