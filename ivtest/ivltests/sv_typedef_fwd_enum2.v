// Check that the base type of an enum can be a forward typedef

module test;

  typedef T1;

  typedef enum T1 {
    A, B
  } T2;

  typedef logic [31:0] T1;

  T2 z;

  initial begin
    if ($bits(z) == 32) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
