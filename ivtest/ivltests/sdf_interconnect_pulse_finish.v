// Finish before a canceled callback and a surviving rearmed callback are due.
// Normal completion is not by itself a heap-leak measurement.
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
  pulse_source source(drive, source_q);
  pulse_consumer consumer(source_q, received);

  always @(received) begin
    if ($time != 10 || received !== 1'b0 || changes != 0)
      $fatal(1, "unexpected receiving event before finish: %0t/%b", $time, received);
    changes = changes + 1;
  end

  initial begin
    $sdf_annotate("ivltests/sdf_interconnect_pulse_finish.sdf", test);
    drive = 0;
    #50;
    if (received !== 1'b0 || changes != 1)
      $fatal(1, "connection did not settle before pending-work test");
    #50 drive = 1;
    #3 drive = 0;
    #3 drive = 1;
    #3;
    if (received !== 1'b0 || source_q !== 1'b1 || changes != 1)
      $fatal(1, "receiving history mismatch at finish");
    $display("PASSED");
    $finish(0);
  end
endmodule
