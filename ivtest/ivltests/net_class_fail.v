// Check that declaring a net of a class type results in an error

module test;
  class C;
  endclass

  wire C x;

  initial begin
    $display("FAILED");
  end
endmodule
