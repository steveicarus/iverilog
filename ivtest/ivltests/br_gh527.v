typedef struct packed {
    union packed {
        logic[2:0] a;
        logic[2:0] b;
    } u;
} s1;

module top();

s1 source;
logic result;

logic failed = 0;

initial begin
  source.u.a = 3'b000;
  result = | source.u.b;
  if (result !== 1'b0) failed = 1;
  source.u.a = 3'b001;
  result = | source.u.b;
  if (result !== 1'b1) failed = 1;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
