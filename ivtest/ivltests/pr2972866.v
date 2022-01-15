`timescale 1ns/10ps

module top;
  reg pass, clk;
  wire out;

  initial begin
    $monitor("%f %b %b", $realtime, out, clk);
    pass = 1'b1;
    $sdf_annotate("ivltests/pr2972866.sdf", dut);
    clk = 1'b0;
    #10 clk = 1'b1;
    #10 clk = 1'b0;
    // Don't check for just PASSED since we are looking for modpath
    // problems (SDF WARNING)!
    #10 if (pass) $display("Simulation ran correctly.");
  end

  always @(out) if (out !== clk && $time != 0) begin
    $display("Failed to match, expected %b, got %b.", clk, out);
    pass = 1'b0;
  end


  ckt dut (out, clk);
endmodule

module ckt(clk_out, clk_in);
  output clk_out;
  input clk_in;
  wire clk_l1;

  CLK_BUF L1 (clk_l1, clk_in);
  CLK_BUF L2 (clk_out, clk_l1);
endmodule

module CLK_BUF(out, in);
  output out;
  input in;

  buf b1 (out, in);

  specify
    (in +=> out) = (0.1:0.1:0.1, 0.1:0.1:0.1);
  endspecify
endmodule
