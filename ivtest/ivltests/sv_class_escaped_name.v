// Check that a class name can contain a double quote.

module test;
  class \C"name ;
    int value;
  endclass

  \C"name obj;

  initial begin
    obj = new;
    obj.value = 23;

    if (obj.value !== 23) $display("FAILED");
    else $display("PASSED");
  end
endmodule
