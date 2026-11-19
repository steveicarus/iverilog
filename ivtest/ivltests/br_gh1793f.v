// Check an unpacked array as an output argument of an automatic task call in
// an automatic calling task, alone and after another output argument that has
// a variable index.

module test;

  integer a [0:3];
  integer whole [0:2];
  integer w2 [0:1][0:1];
  reg failed;

  task automatic fill(input integer i, output integer o, output integer arr[3]);
    o = i;
    arr[0] = 1;
    arr[1] = 2;
    arr[2] = 3;
  endtask

  task automatic fill2(output integer arr[2][2]);
    arr[0][0] = 1;
    arr[1][1] = 4;
  endtask

  task automatic run(input integer at);
    integer loc;
    fill(5, a[at], whole);
    if (a[1] !== 5 || whole[0] !== 1 || whole[1] !== 2 || whole[2] !== 3) begin
      $display("FAILED: %0d %0d %0d %0d", a[1], whole[0], whole[1], whole[2]);
      failed = 1'b1;
    end
    whole[0] = 0;
    whole[1] = 0;
    whole[2] = 0;
    fill(6, loc, whole);
    if (loc !== 6 || whole[0] !== 1 || whole[1] !== 2 || whole[2] !== 3) begin
      $display("FAILED: %0d %0d %0d %0d", loc, whole[0], whole[1], whole[2]);
      failed = 1'b1;
    end
    fill2(w2);
    if (w2[0][0] !== 1 || w2[1][1] !== 4) begin
      $display("FAILED: %0d %0d", w2[0][0], w2[1][1]);
      failed = 1'b1;
    end
  endtask

  initial begin
    failed = 1'b0;
    run(1);
    if (!failed) $display("PASSED");
  end

endmodule
