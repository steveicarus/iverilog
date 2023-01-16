// Check that passing an enum type task argument works.

module test;

  typedef enum reg [31:0] {
    A, B
  } E;

  task t(E e);
    if (e === B) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  endtask

  initial begin
    t(B);
  end

endmodule
