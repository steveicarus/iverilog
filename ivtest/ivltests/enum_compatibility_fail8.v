// Check that an error is reported for implicit cast to enum in function return
// statements.

module test;

  typedef enum reg [31:0] {
    A, B
  } E;

  function E f();
    return 10; // This should fail. Implicit cast to enum.
  endfunction

  E e;

  initial begin
    e = f();
    $display("FAILED");
  end

endmodule
