// Check that using a class type as the base type for an enum results in an
// error.

class C;
endclass

module test;

  enum C {
    A
  } e;

  initial begin
    $display("FAILED");
  end

endmodule
