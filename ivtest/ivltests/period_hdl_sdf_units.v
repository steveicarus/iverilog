// Declaring-module limits are in ns; the testbench and SDF are in ps.
`timescale 1ns/1ps
module period_local;
  reg \clock.name ;
  reg \notifier.name ;
  reg low_notifier;
  period_history_check #(1, 105) high_history(\notifier.name );
  period_history_check #(1, 107) low_history(low_notifier);

  initial begin
    \notifier.name = 0;
    low_notifier = 0;
  end
  specify
    $period(posedge \clock.name , 0.010, \notifier.name );
    $period(negedge \clock.name , 0.010);
    $period(negedge \clock.name , 0.010, low_notifier);
  endspecify
  initial begin
    // The second selected edge violates on both streams at different times.
    \clock.name = 0;
    #0.100 \clock.name = 1;
    #0.002 \clock.name = 0;
    #0.003 \clock.name = 1;
    #0.002 \clock.name = 0;
    #0.093 \clock.name = 1;
    #0.002 \clock.name = 0;
    #0.010 \clock.name = 1;
    #0.002 \clock.name = 0;
  end
endmodule

module period_unannotated(input wire \clock.name );
  reg notifier;
  period_history_check #(1, 124) history(notifier);
  initial notifier = 0;
  specify
    $period(posedge \clock.name , 0.010, notifier);
  endspecify
endmodule

module period_path(\clock.name , Q);
  input \clock.name ;
  output Q;
  reg \notifier.name ;
  buf (Q, \clock.name );
  period_history_check #(2, 115, 124) history(\notifier.name );

  initial \notifier.name = 0;
  specify
    (\clock.name => Q) = (0.003, 0.003);
    $period(posedge \clock.name , 0.010, \notifier.name );
    $period(negedge \clock.name , 0.010);
  endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg CLK;
  wire Q;
  integer q_events, expected_time;
  period_local local_check();
  period_unannotated default_leaf(CLK);
  period_path annotated(CLK, Q);

  // The annotated IOPATH changes the original 3 ps delay to 1 ps.
  always @(Q) if ($time > 50) begin
    case (q_events)
      0: expected_time = 101;
      1: expected_time = 103;
      2: expected_time = 116;
      3: expected_time = 118;
      4: expected_time = 125;
      5: expected_time = 127;
      6: expected_time = 145;
      7: expected_time = 147;
      default: expected_time = -1;
    endcase
    if ($time != expected_time || Q !== (q_events % 2 == 0)) begin
      $display("FAILED annotated IOPATH event %0d at %0t", q_events, $time);
      $finish(1);
    end
    q_events = q_events + 1;
  end
  initial begin
    // Successive rising edges are 15, 9 and 20 ps apart.
    CLK = 0;
    #100 CLK = 1; #2 CLK = 0;
    #13 CLK = 1; #2 CLK = 0;
    #7 CLK = 1;  #2 CLK = 0;
    #18 CLK = 1; #2 CLK = 0;
  end
  initial begin
    q_events = 0;
    #5 $sdf_annotate("ivltests/period_hdl_sdf_units.sdf", annotated);
    $period_limit("tb.default_leaf", "period", "clock.name", "posedge", 0.010, 10);
    $period_limit("tb.annotated", "period", "clock.name", "posedge", 0.020, 20);
    $period_limit("tb.annotated", "period", "clock.name", "negedge", 0.010, 10);
    #315;
    local_check.high_history.complete;
    local_check.low_history.complete;
    default_leaf.history.complete;
    annotated.history.complete;
    if (q_events != 8) begin
      $display("FAILED annotated IOPATH count %0d", q_events);
      $finish(1);
    end
    $display("PASSED");
    $finish(0);
  end
endmodule

`include "period_history_check.vh"
