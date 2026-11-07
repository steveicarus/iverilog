// Portless scope, escaped scalar names, fractional limits and cold SDF.
// Direct notifier monitors avoid hierarchical module-port expressions.
`timescale 1ns/1ps
module target_plain;
  reg \clock.name ;
  reg \notifier.name ;
  integer seen;
  reg negedge_notifier;
  integer neg_seen;
  initial begin
    \notifier.name = 0;
    seen = 0;
    negedge_notifier = 0;
    neg_seen = 0;
  end
  always @(\notifier.name ) if ($realtime > 0.0) begin
    if ((seen == 0 && ($realtime < 0.1049 || $realtime > 0.1051 ||
                       \notifier.name !== 1'b1)) ||
        (seen == 1 && ($realtime < 0.2119 || $realtime > 0.2121 ||
                       \notifier.name !== 1'b0)) || seen > 1) begin
      $display("FAILED target notifier history %m at %g", $realtime);
      $finish(1);
    end
    seen = seen + 1;
  end
  always @(negedge_notifier) if ($realtime > 0.0) begin
    if (neg_seen != 0 || $realtime < 0.1069 || $realtime > 0.1071 ||
        negedge_notifier !== 1'b1) begin
      $display("FAILED target negedge history %m at %g", $realtime);
      $finish(1);
    end
    neg_seen = neg_seen + 1;
    $display("TARGET_NEGEDGE PERIOD time=%g value=%b", $realtime,
             negedge_notifier);
  end
  specify
    $period(posedge \clock.name , 0.010, \notifier.name );
    $period(negedge \clock.name , 0.010);
    $period(negedge \clock.name , 0.010, negedge_notifier);
  endspecify
  initial begin
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

`timescale 1ns/1ps
module target_annotated(\clock.name , Q);
  input \clock.name ;
  output Q;
  buf (Q, \clock.name );
  reg \notifier.name ;
  integer seen;
  initial begin
    \notifier.name = 0;
    seen = 0;
  end
  always @(\notifier.name ) if ($realtime > 0.0) begin
    if ((seen == 0 && ($realtime < 0.1049 || $realtime > 0.1051 ||
                       \notifier.name !== 1'b1)) ||
        (seen == 1 && ($realtime < 0.2119 || $realtime > 0.2121 ||
                       \notifier.name !== 1'b0)) || seen > 1) begin
      $display("FAILED target notifier history %m at %g", $realtime);
      $finish(1);
    end
    seen = seen + 1;
  end
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
  integer qseen, expected_time, result_file;
  target_plain plain();
  target_annotated annotated(CLK, Q);
  always @(Q) if ($time > 50) begin
    case (qseen)
      0: expected_time = 101;
      1: expected_time = 103;
      2: expected_time = 106;
      3: expected_time = 108;
      4: expected_time = 201;
      5: expected_time = 203;
      6: expected_time = 213;
      7: expected_time = 215;
      default: expected_time = -1;
    endcase
    if ($time != expected_time || Q !== (qseen % 2 == 0)) begin
      $display("FAILED target IOPATH event %0d at %0t", qseen, $time);
      $finish(1);
    end
    qseen = qseen + 1;
  end
  initial begin
    CLK = 0;
    #100 CLK = 1;
    #2 CLK = 0;
    #3 CLK = 1;
    #2 CLK = 0;
    #93 CLK = 1;
    #2 CLK = 0;
    #10 CLK = 1;
    #2 CLK = 0;
  end
  initial begin
    qseen = 0;
    result_file = $fopen("work/period_hdl_vlog95.out");
    #1 $sdf_annotate("ivltests/period_hdl_vlog95.sdf", annotated);
    #319;
    if (plain.seen != 1 || plain.neg_seen != 1 || annotated.seen != 2 || qseen != 8) begin
      $display("FAILED target final counts pos=%0d neg=%0d sdf=%0d q=%0d",
               plain.seen, plain.neg_seen, annotated.seen, qseen);
      $finish(1);
    end
    $fdisplay(result_file, "PASSED");
    $fclose(result_file);
    $display("PASSED");
  end
endmodule
