`timescale 1ps/1ps
module driver(input [1:0] I, output [1:0] Q);
  assign Q = I;
endmodule
module receiver(input I, output Q);
  buf (Q, I);
endmodule
module top;
  reg [1:0] drive;
  wire [1:0] source_q;
  wire q0, q1;
  integer count0 = 0, count1 = 0;
  driver source(drive, source_q);
  receiver left(source_q[0], q0);
  receiver right(source_q[1], q1);
  initial begin
    $sdf_annotate("vpi/interconnect_reuse_bits.sdf", top);
    $interconnect_reuse_bits;
    drive = 0;
    #100 drive = 3;
    #50 $interconnect_reuse_bits;
    #50 drive = 0;
    #50;
    if (count0 != 2 || count1 != 2)
      $fatal(1, "indexed delivery counts: %0d %0d", count0, count1);
    $display("PASS: independent indexed deliveries");
    $finish(0);
  end
  always @(q0) if ($time > 50) begin
    count0 = count0 + 1;
    if (!((q0 === 1'b1 && $time == 103) ||
          (q0 === 1'b0 && $time == 205)))
      $fatal(1, "indexed bit0 delivery at %0t", $time);
  end
  always @(q1) if ($time > 50) begin
    count1 = count1 + 1;
    if (!((q1 === 1'b1 && $time == 107) ||
          (q1 === 1'b0 && $time == 211)))
      $fatal(1, "indexed bit1 delivery at %0t", $time);
  end
endmodule
