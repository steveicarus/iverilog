// Check that declaring a net of string type results in an error

module test;

  wire string x;

  initial begin
    $display("FAILED");
  end
endmodule
