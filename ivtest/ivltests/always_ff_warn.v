module top;
  reg a;
  reg q, d;
  reg clk;
  event foo;
  real rl;
  int ar [];
  int start = 0;
  int stop = 1;
  int step = 1;
  int done = 0;

  task a_task;
    real trl;
    event tevt;
    reg tvr;
    $display("user task");
  endtask

  always_ff @(posedge clk)  begin: blk_name
    event int1, int2;
    real intrl;
    q = d;
    -> foo;
    rl = 0.0;
    rl <= 1.0;
    ar = new [2];
    for (int idx = start; idx < stop; idx += step) $display("For: %0d", idx);
    for (int idx = 0; done; idx = done + 1) $display("Should never run!");
    for (int idx = 0; idx; done = done + 1) $display("Should never run!");
    for (int idx = 0; idx; {done, idx} = done + 1) $display("Should never run!");
    for (int idx = 0; idx; idx <<= 1) $display("Should never run!");
    for (int idx = 0; idx; idx = idx << 1) $display("Should never run!");
    $display("array size: %0d", ar.size());
    ar.delete();
    $display("array size: %0d", ar.size());
    a_task;
    assign a = 1'b0;
    deassign a;
    do $display("do/while");
    while (a);
    force a = 1'b1;
    release a;
    while(a) begin
      $display("while");
      a = 1'b0;
    end
    repeat(2) $display("repeat");
    disable out_name;
    forever begin
      $display("forever");
      disable blk_name; // This one should not generate a warning
    end
  end

  initial begin
    #1 clk = 1'b1;
    #0 $display("Expect compile warnings!\nPASSED");
  end

  initial begin: out_name
    #2 $display("FAILED");
  end

endmodule
