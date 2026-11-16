// A class can not extend a class with parameters that have no default value
// without giving them a value.

class C #(type T, int N);
  T data;
endclass

class D extends C;
endclass

module test;

  initial begin
    $display("FAILED");
  end

endmodule
