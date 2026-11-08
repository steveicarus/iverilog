// The trailing delay supplies the rejection limit; equality survives.
`timescale 1ps/1ps

module pulse_source(input drive, output Q);
  assign Q = drive;
endmodule

module pulse_consumer(input I, output Q);
  assign Q = I;
endmodule

module test;
  reg drive;
  wire source_q, received;
  integer changes = 0;
  reg failed = 0;
  pulse_source source(drive, source_q);
  pulse_consumer consumer(source_q, received);

  task check;
    input time expected_time;
    input expected_value;
    begin
      if ($time != expected_time || received !== expected_value) begin
        $display("receiving change %0d: got %0t/%b, expected %0t/%b",
                 changes, $time, received, expected_time, expected_value);
        failed = 1;
      end
    end
  endtask

  always @(received) begin
    case (changes)
      0: check(13, 0);
      1: check(107, 1);   // control proves both delays, 7 and 13
      2: check(153, 0);
      3: check(213, 1);   // nominal 207/216 pulse rejected; new rise at 213
      4: check(253, 0);
                          // nominal 307/319 width 12 < 13: reject
      5: check(407, 1);   // nominal 407/420 width 13 == 13: retain
      6: check(420, 0);
      default: begin
        $display("extra receiving change %0d at %0t: %b",
                 changes, $time, received);
        failed = 1;
      end
    endcase
    changes = changes + 1;
  end

  initial begin
    $sdf_annotate("ivltests/sdf_interconnect_pulse_asymmetric.sdf", test);
    drive = 0;
    #50;
    if (received !== 1'b0 || changes != 1)
      $fatal(1, "asymmetric connection did not settle");
    #50 drive = 1;
    #40 drive = 0;
    #60 drive = 1;
    #3 drive = 0;
    #3 drive = 1;
    #34 drive = 0;
    #60 drive = 1;
    #6 drive = 0;
    #94 drive = 1;
    #7 drive = 0;         // input return coincides with the leading due time
    #93;
    if (failed || changes != 7 || received !== 1'b0 || source_q !== 1'b0)
      $fatal(1, "asymmetric INTERCONNECT history mismatch");
    $display("PASSED");
    $finish(0);
  end
endmodule
