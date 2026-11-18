// Declaring-module limits are in ns; the testbench and SDF are in ps.
`timescale 1ns/1ps
module width_default;
  reg \clock.name ;
  reg \notifier.name ;
  reg low_notifier;
  width_history_check #(1, 209) high_history(\notifier.name );
  width_history_check #(1, 300) low_history(low_notifier);

  initial begin
    \notifier.name = 0;
    low_notifier = 0;
  end
  specify
    $width(posedge \clock.name , 0.010, 0.002, \notifier.name );
    $width(posedge \clock.name , 0.010);
    $width(negedge \clock.name , 0.100, 0.090, low_notifier);
    $width(negedge \clock.name , 0.100, 0.090);
  endspecify
  initial begin
    // High pulses are 15, 9, 20 and 2 ps. Only the 91 ps low pulse
    // falls strictly between the 90 ps threshold and the 100 ps limit.
    \clock.name = 0;
    #0.100 \clock.name = 1;
    #0.015 \clock.name = 0;
    #0.085 \clock.name = 1;
    #0.009 \clock.name = 0;
    #0.091 \clock.name = 1;
    #0.020 \clock.name = 0;
    #0.080 \clock.name = 1;
    #0.002 \clock.name = 0;
  end
endmodule

module width_annotated(\clock.name , Q);
  input \clock.name ;
  output Q;
  reg \notifier.name ;
  buf (Q, \clock.name );
  width_history_check #(2, 115, 209) high_history(\notifier.name );

  initial \notifier.name = 0;
  specify
    (\clock.name => Q) = (0.003, 0.003);
    $width(posedge \clock.name , 0.010, 0.002, \notifier.name );
  endspecify
endmodule

`timescale 1ps/1ps
module tb;
  reg CLK;
  wire Q;
  integer q_events, expected_time;
  width_default plain();
  width_annotated annotated(CLK, Q);

  // The annotated IOPATH changes the original 3 ps delay to 1 ps.
  always @(Q) if ($time > 50) begin
    case (q_events)
      0: expected_time = 101;
      1: expected_time = 116;
      2: expected_time = 201;
      3: expected_time = 210;
      4: expected_time = 301;
      5: expected_time = 321;
      6: expected_time = 401;
      7: expected_time = 403;
      default: expected_time = -1;
    endcase
    if ($time != expected_time || Q !== (q_events % 2 == 0)) begin
      $display("FAILED annotated IOPATH event %0d at %0t", q_events, $time);
      $finish(1);
    end
    q_events = q_events + 1;
  end
  initial begin
    CLK = 0;
    #100 CLK = 1;
    #15 CLK = 0;
    #85 CLK = 1;
    #9 CLK = 0;
    #91 CLK = 1;
    #20 CLK = 0;
    #80 CLK = 1;
    #2 CLK = 0;
  end
  initial begin
    q_events = 0;
    #1 $sdf_annotate("ivltests/width_hdl_sdf_units.sdf", annotated);
    #449;
    plain.high_history.complete;
    plain.low_history.complete;
    annotated.high_history.complete;
    if (q_events != 8) begin
      $display("FAILED annotated IOPATH count %0d", q_events);
      $finish(1);
    end
    $display("PASSED");
    $finish(0);
  end
endmodule

`include "width_history_check.vh"
