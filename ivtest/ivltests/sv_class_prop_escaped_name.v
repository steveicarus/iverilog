// Check that a class property name can contain a double quote.

module test;
  class C;
    int \value"name ;
  endclass

  C obj;

  initial begin
    obj = new;
    obj.\value"name = 23;

    if (obj.\value"name !== 23) $display("FAILED");
    else $display("PASSED");
  end
endmodule
