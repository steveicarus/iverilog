// Check const property initialization in a nested constructor block.

module test;

  class C;
    const int value;

    function new;
      begin : initialize
        value = 42;
      end
    endfunction
  endclass

  C object;

  initial begin
    object = new;

    if (object.value !== 42) begin
      $display("FAILED: expected 42, got %0d", object.value);
    end else begin
      $display("PASSED");
    end
  end

endmodule
