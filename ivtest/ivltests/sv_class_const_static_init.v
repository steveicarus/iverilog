// Check initialization of a const static class property.

module test;

  class C;
    const static int value = 42;
  endclass

  C object;

  initial begin
    // A static property can be accessed without constructing the object.
    if (object.value !== 42) begin
      $display("FAILED: expected 42, got %0d", object.value);
    end else begin
      $display("PASSED");
    end
  end

endmodule
