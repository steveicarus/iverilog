`timescale 1ns/1ns

module top;
  wire q;
  reg a, b;

  initial begin
//    $dumpfile("test.lx2");  // Need to also use the -lxt2 flags on exe.
//    $dumpvars(0, top);
                            // Initial value should be X is 1.
    #1 a = 1'b1;            // Should be X is 1.
    #1 if (q !== 1'bx) begin
       $display("FAILED -- %b nand %b --> %b", a, b, q);
       $finish;
    end
    #1 a = 1'b0;            // Correct: 1.
    #1 if (q !== 1'b1) begin
       $display("FAILED -- %b nand %b --> %b", a, b, q);
       $finish;
    end
    #1 a = 1'b1;            // Should be X is 1.
    #1 if (q !== 1'bx) begin
       $display("FAILED -- %b nand %b --> %b", a, b, q);
       $finish;
    end
    #1 a = 1'bx;            // Should be X is 1.
    #1 if (q !== 1'bx) begin
       $display("FAILED -- %b nand %b --> %b", a, b, q);
       $finish;
    end
    #1 a = 1'b1;            // Should be X is 1.
    #1 if (q !== 1'bx) begin
       $display("FAILED -- %b nand %b --> %b", a, b, q);
       $finish;
    end
    #1 a = 1'bx; b = 1'b1;  // Correct: X.
    #1 if (q !== 1'bx) begin
       $display("FAILED -- %b nand %b --> %b", a, b, q);
       $finish;
    end
    #1 b = 1'b0;            // Correct: 1.
    #1 if (q !== 1'b1) begin
       $display("FAILED -- %b nand %b --> %b", a, b, q);
       $finish;
    end
    #1 b = 1'b1;            // Correct: X, but following #1 delay is missing.
    #1 if (q !== 1'bx) begin
       $display("FAILED -- %b nand %b --> %b", a, b, q);
       $finish;
    end
    $display("PASSED");
    $finish;
  end

  nand dut (q, a, b);
//  nand dut (q, b, a);       // This also produces incorrect results.
endmodule
