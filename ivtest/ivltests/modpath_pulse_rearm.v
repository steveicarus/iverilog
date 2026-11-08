// SourceForge #888: a canceled pulse must not deliver a later request early.
// The rising requests at 1 ns and 3 ns have distinct due times, 5 ns and 7 ns.
`timescale 1ps/1ps

module rearm_path(input i, output o);
  assign o = i;
  specify
    (i => o) = (4000, 0);
  endspecify
endmodule

module test;
  reg i;
  wire o, reference;
  integer path_changes = 0;
  integer reference_changes = 0;
  reg failed = 0;

  rearm_path dut(i, o);
  assign #(4000, 0) reference = i;

  task check_change;
    input [8*20-1:0] channel;
    input integer index;
    input value;
    reg mismatch;
    begin
      mismatch = 0;
      case (index)
        0: if ($time != 0 || value !== 1'b0) mismatch = 1;
        1: if ($time != 7000 || value !== 1'b1) mismatch = 1;
        default: mismatch = 1;
      endcase
      if (mismatch) begin
        failed = 1;
        $display("%0s unexpected change %0d at %0t: %b",
                 channel, index, $time, value);
      end
    end
  endtask

  always @(o) begin
    check_change("module-path", path_changes, o);
    path_changes = path_changes + 1;
  end
  always @(reference) begin
    check_change("delayed-reference", reference_changes, reference);
    reference_changes = reference_changes + 1;
  end

  initial begin
    i = 0;
    #1000 i = 1;
    #1000 i = 0;
    #1000 i = 1;
    #9000;
    if (failed || path_changes != 2 || reference_changes != 2 ||
        o !== 1'b1 || reference !== 1'b1)
      $fatal(1, "pulse rearming history mismatch: path count/value %0d/%b, reference %0d/%b",
             path_changes, o, reference_changes, reference);
    $display("PASSED");
    $finish(0);
  end
endmodule
