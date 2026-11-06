`timescale 1ns/1ns

module timed_buffer(input a, en, output y);
  buf (y, a);
  specify
    if (en) (posedge a => (y : a)) = 11;
    if (en) (negedge a => (y : a)) = 3;
  endspecify
endmodule

module test;
  reg a;
  wire y;
  integer events = 0;
  timed_buffer dut(a, 1'b1, y);

  initial begin
    // Let port connections settle, then produce only a posedge at time zero.
    #0 a = 1;
    #40 a = 0;
    #40 a = 1;
    #40;
    if (events != 3) $fatal(1, "Expected three output events, got %0d", events);
    $display("PASSED");
  end

  always @(y) begin
    events = events + 1;
    case (events)
      1: if ($time != 11 || y !== 1'b1) $fatal(1, "First posedge: y=%b at %0t", y, $time);
      2: if ($time != 43 || y !== 1'b0) $fatal(1, "First negedge: y=%b at %0t", y, $time);
      3: if ($time != 91 || y !== 1'b1) $fatal(1, "Next posedge: y=%b at %0t", y, $time);
      default: $fatal(1, "Unexpected output event: y=%b at %0t", y, $time);
    endcase
  end
endmodule
