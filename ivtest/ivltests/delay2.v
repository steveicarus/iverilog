/*
 * This program is derived from iverilog issue # 1327436.
 */

`timescale 1ns/1ns
module verilog_test ();

reg [24:0] APAD;
wire [24:0] AIN;

initial begin
//  $dumpfile("dumpfile.vcd");
//  $dumpvars;

  APAD=25'h1ffffff;
  #21 if (AIN !== APAD) begin
     $display("FAILED -- APAD=%b, AIN=%b, time=%0t", APAD, AIN, $time);
     $finish;
  end

  #79
  APAD=25'h1555555;

  #19 if (AIN !== 25'h1ffffff) begin
     $display("FAILED --  APAD=%b, AIN=%b, time=%0t", APAD, AIN, $time);
     $finish;
  end
  #2 if (AIN !== 25'h1555555) begin
     $display("FAILED --  APAD=%b, AIN=%b, time=%0t", APAD, AIN, $time);
     $finish;
  end

  #79
  APAD=25'h0aaaaaa;

  #19 if (AIN !== 25'h1555555) begin
     $display("FAILED --  APAD=%b, AIN=%b, time=%0t", APAD, AIN, $time);
     $finish;
  end
  #2 if (AIN !== 25'h0aaaaaa) begin
     $display("FAILED --  APAD=%b, AIN=%b, time=%0t", APAD, AIN, $time);
     $finish;
  end

  #79
  APAD=25'h1555555;

  #19 if (AIN !== 25'h0aaaaaa) begin
     $display("FAILED --  APAD=%b, AIN=%b, time=%0t", APAD, AIN, $time);
     $finish;
  end
  #2 if (AIN !== 25'h1555555) begin
     $display("FAILED --  APAD=%b, AIN=%b, time=%0t", APAD, AIN, $time);
     $finish;
  end

  #79
  APAD=25'h0aaaaaa;

  #19 if (AIN !== 25'h1555555) begin
     $display("FAILED --  APAD=%b, AIN=%b, time=%0t", APAD, AIN, $time);
     $finish;
  end
  #2 if (AIN !== 25'h0aaaaaa) begin
     $display("FAILED --  APAD=%b, AIN=%b, time=%0t", APAD, AIN, $time);
     $finish;
  end

  $display("PASSED");
end

assign #20 AIN= APAD;
endmodule
