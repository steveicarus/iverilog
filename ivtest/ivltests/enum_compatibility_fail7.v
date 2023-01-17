// Check that an error is reported for implicit cast to enum when assigning to
// class properties.

module test;

  typedef enum reg [31:0] {
    A, B
  } E;

  class C;
    E e;
  endclass

  C c;

  initial begin
    c = new;
    c.e = 10; // This should fail. Implicit cast to enum.
    $display("FAILED");
  end

endmodule
