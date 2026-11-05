// Extended version of test case from issue #1652.

module test;

typedef struct packed { logic [7:0] d; logic k; } w_t;

w_t w[4];
logic [7:0] i;
bit failed = 0;

initial begin
  for (i = 0; i < 4; i++) begin
    w[i].d = i;
  end
  for (i = 0; i < 4; i++) begin
    $display("%b %h", w[i].k, w[i].d);
    if (w[i].k !== 1'bx || w[i].d !== i) failed = 1;
  end
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
