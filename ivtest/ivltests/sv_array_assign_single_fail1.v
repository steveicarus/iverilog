// Check that scalar expressions can not be procedurally assigned to single
// element unpacked arrays.

module test;

  integer a[0:0];

  initial begin
    a = 1;
    $display("FAILED");
  end

endmodule
