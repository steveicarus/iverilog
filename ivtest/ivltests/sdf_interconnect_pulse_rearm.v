// A canceled INTERCONNECT event must not deliver a later request early.
`timescale 1ps/1ps

module pulse_source(input drive, output Q);
  assign Q = drive;
endmodule

module pulse_consumer(input I, output Q);
  // No cell delay or specify path: Q exposes the delayed receiving value.
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
      0: check(10, 0);    // annotation precedes source initialization
      1: check(110, 1);   // ordinary long pulse
      2: check(150, 0);
                          // short pulse at 200/203 produces no output
      3: check(316, 1);   // rearmed rise has its own due time
      4: check(350, 0);
      5: check(410, 1);   // prepare opposite polarity
      6: check(456, 0);   // rearmed fall
      7: check(522, 1);   // repeated cancellation, then persistent rise
      default: begin
        $display("extra receiving change %0d at %0t: %b",
                 changes, $time, received);
        failed = 1;
      end
    endcase
    changes = changes + 1;
  end

  initial begin
    $sdf_annotate("ivltests/sdf_interconnect_pulse_rearm.sdf", test);
    drive = 0;
    #50;
    if (received !== 1'b0 || source_q !== 1'b0 || changes != 1)
      $fatal(1, "annotated connection did not settle");
    #50 drive = 1;         // 100: control
    #40 drive = 0;
    #60 drive = 1;         // 200: short return
    #3 drive = 0;
    #97 drive = 1;         // 300: rearm
    #3 drive = 0;
    #3 drive = 1;
    #34 drive = 0;
    #60 drive = 1;         // 400: prepare opposite polarity
    #40 drive = 0;
    #3 drive = 1;
    #3 drive = 0;
    #54 drive = 1;         // 500: repeated cancellation
    #3 drive = 0;
    #3 drive = 1;
    #3 drive = 0;
    #3 drive = 1;
    #8 drive = 1;          // unchanged assignment must not rearm
    #80;
    if (failed || changes != 8 || received !== 1'b1 || source_q !== 1'b1)
      $fatal(1, "INTERCONNECT pulse rearming history mismatch");
    $display("PASSED");
    $finish(0);
  end
endmodule
