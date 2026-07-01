// Check that function context is preserved when elaborating nested blocks.

module test;

  reg x;
  integer y;

  function integer f;
    input a;
    begin : nested
      x <= a;
      f = 0;
    end
  endfunction

  initial begin
    y = f(1'b1);
    $display("FAILED");
  end

endmodule
