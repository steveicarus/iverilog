// Check that declaring a net of a queue type results in an error

module test;

  wire x[$];

  initial begin
    $display("FAILED");
  end
endmodule
