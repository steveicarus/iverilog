// Check that an unexecuted property read through a null handle causes no error.

class C;
  int value;
endclass

module test;
  C object;
  bit read_property;

  initial begin
    if (read_property) $display("%0d", object.value);
    $display("PASSED");
  end
endmodule
