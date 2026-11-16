// Check output arguments with variable indexes in other contexts: a
// recursive automatic task, a named block with local variables, a static
// calling task, a queue element, a dynamic array element and a class property.

module test;

  class C;
    int x;
  endclass

  reg [7:0] mem [0:7];
  int q[$];
  int da[];
  reg failed;

  task automatic put8(input [7:0] a, output [7:0] v);
    v = a;
  endtask

  task automatic put_int(input integer a, output integer v);
    v = a;
  endtask

  task automatic rec(input integer depth);
    if (depth < 4) begin
      put8(8'h10 + depth, mem[depth]);
      rec(depth + 1);
    end
  endtask

  task automatic in_block(input integer at);
    begin : blk
      integer j;
      j = at + 1;
      put8(8'h30, mem[j]);
    end
  endtask

  task static_caller;
    integer k;
    k = 5;
    put8(8'h40, mem[k]);
  endtask

  task automatic objects(input integer at);
    C o;
    o = new;
    put_int(7, q[at]);
    put_int(9, da[at]);
    put_int(5, o.x);
    if (o.x !== 5) begin
      $display("FAILED: property is %0d", o.x);
      failed = 1'b1;
    end
  endtask

  integer idx;

  initial begin
    failed = 1'b0;
    rec(0);
    for (idx = 0; idx < 4; idx = idx + 1)
      if (mem[idx] !== 8'h10 + idx) begin
        $display("FAILED: mem[%0d] is %h", idx, mem[idx]);
        failed = 1'b1;
      end

    in_block(1);
    if (mem[2] !== 8'h30) begin
      $display("FAILED: mem[2] is %h", mem[2]);
      failed = 1'b1;
    end

    static_caller;
    if (mem[5] !== 8'h40) begin
      $display("FAILED: mem[5] is %h", mem[5]);
      failed = 1'b1;
    end

    q = '{0, 0, 0};
    da = new[3];
    objects(1);
    if (q[1] !== 7 || q[0] !== 0 || q[2] !== 0) begin
      $display("FAILED: queue is %p", q);
      failed = 1'b1;
    end
    if (da[1] !== 9 || da[0] !== 0 || da[2] !== 0) begin
      $display("FAILED: dynamic array is %p", da);
      failed = 1'b1;
    end

    if (!failed) $display("PASSED");
  end

endmodule
