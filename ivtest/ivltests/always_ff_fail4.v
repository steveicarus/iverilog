module top;
  reg a, b;
  reg q, d;
  reg clk;
  event foo;

  always_ff @(posedge clk) begin
    q <= d;
    fork
      $display("fork/join 1");
    join
    fork
      $display("fork/join_any 1");
    join_any
    fork
      $display("fork/join_none 1");
    join_none
    a <= @foo 1'b1;
    @(b) a <= repeat(2) @foo 1'b0;
    wait (!a) $display("wait");
  end

  initial #1 $display("Expect compile errors!");

endmodule
