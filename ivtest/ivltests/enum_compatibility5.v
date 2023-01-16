// Check that assigning to a enum types variable from an enum typed function
// call works.

module test;

  typedef enum reg [31:0] {
    A, B
  } E;

  function E f();
    return B;
  endfunction

  E e;

  initial begin
    e = f();
    if (e === B) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
