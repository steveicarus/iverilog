// Check that declaring a net of a dynamic array type results in an error

module test;

  wire x[];

  initial begin
    $display("FAILED");
  end
endmodule
