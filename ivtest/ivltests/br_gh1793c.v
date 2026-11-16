// Check inout arguments and the evaluation time of the l-value of an output
// argument. The l-value is evaluated when the called task returns, in the
// order of the arguments, for automatic and static calling tasks alike.

module test;

  reg [7:0] mem [0:7];
  reg [7:0] mid;
  integer k_after;
  reg failed;

  task automatic bump(inout [7:0] v);
    v = v + 8'h01;
  endtask

  task automatic set_both(output integer n, output [7:0] v);
    n = 3;
    v = 8'h5a;
  endtask

  task automatic set_swapped(output [7:0] v, output integer n);
    v = 8'ha5;
    n = 6;
  endtask

  task automatic auto_caller(input integer at);
    integer k;
    mem[at+1] = 8'h10;
    bump(mem[at+1]);
    mid = mem[at+1];
    k = 1;
    set_both(k, mem[k]);
    k = 0;
    set_swapped(mem[k], k);
    k_after = k;
  endtask

  task static_caller;
    integer k;
    k = 1;
    set_both(k, mem[k+4]);
  endtask

  initial begin
    failed = 1'b0;
    auto_caller(2);
    if (mem[3] !== 8'h5a) begin
      $display("FAILED: mem[3] is %h", mem[3]);
      failed = 1'b1;
    end
    if (mid !== 8'h11) begin
      $display("FAILED: inout result is %h", mid);
      failed = 1'b1;
    end
    if (mem[0] !== 8'ha5 || k_after !== 6) begin
      $display("FAILED: mem[0] is %h", mem[0]);
      failed = 1'b1;
    end
    static_caller;
    if (mem[7] !== 8'h5a) begin
      $display("FAILED: mem[7] is %h", mem[7]);
      failed = 1'b1;
    end
    if (!failed) $display("PASSED");
  end

endmodule
