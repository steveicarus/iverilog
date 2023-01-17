// Check that an error is reported for implicit cast to enum when assigning
// from a function call.

module test;

  typedef enum reg [31:0] {
    A, B
  } E;

  function integer f();
    return B;
  endfunction

  E e;

  initial begin
    e = f(); // This should fail. Implicit cast to enum type.
    $display("FAILED");
  end

endmodule
