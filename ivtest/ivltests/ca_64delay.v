/*
 * Verify that the continuous assignments support a delay that is
 * greater than 32 bits. The top delays are in seconds and the other
 * delays are in ps. The second delays all require more than 32 bits
 * to work correctly. They will use the /d version.
 */

`timescale 1s/1s
module gt32b;
  wire real rlval;
  wire rval;
  wire aval[1:0];
  wire [7:0] psval;

  assign #1 rlval = 1.0;
  assign #2 rval = 1'b1;
  assign #3 aval[0] = 1'b0;
  assign #4 psval[1] = 1'b1;

  initial begin
    $timeformat(-12, 0, " ps", 16);
    #1;
    $display("dl:gt32b- %t", $realtime);
  end

  always @(rlval) begin
    $display("rl:gt32b- %t", $realtime);
  end

  always @(rval) begin
    $display("rg:gt32b- %t", $realtime);
  end

  always @(aval[0]) begin
    $display("ar:gt32b- %t", $realtime);
  end

  always @(psval) begin
    $display("ps:gt32b- %t", $realtime);
  end
endmodule

`timescale 1ps/1ps
module ls32b;
  wire real rlval;
  wire rval;
  wire aval[1:0];
  wire [7:0] psval;

  assign #1 rlval = 1.0;
  assign #2 rval = 1'b1;
  assign #3 aval[0] = 1'b0;
  assign #4 psval[1] = 1'b1;

  initial begin
    #1;
    $display("dl:ls32b- %t", $realtime);
  end

  always @(rlval) begin
    $display("rl:ls32b- %t", $realtime);
  end

  always @(rval) begin
    $display("rg:ls32b- %t", $realtime);
  end

  always @(aval[0]) begin
    $display("ar:ls32b- %t", $realtime);
  end

  always @(psval) begin
    $display("ps:ls32b- %t", $realtime);
  end
endmodule
