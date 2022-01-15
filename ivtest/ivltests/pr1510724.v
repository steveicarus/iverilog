/*
 * This test is really about the defparam working so it just does a
 * simple test of the logic functionality (eveything in manyNands
 * is a two bit bus).
 */

module top;
  parameter count = 2;

  reg pass = 1'b1;
  wire [count-1:0] y;
  reg [count-1:0] a, b;

  manyNands nandArray(y, a, b);
  defparam nandArray.count = count;

  initial begin
    a = 2'b00; b = 2'b00;
    #1;
    if (y !== 2'b11) begin
      $display("FAILED: ~(2'b00 & 2'b00) should be 2'b11, got %b", y);
      pass = 1'b0;
    end

    a = 2'b11; b = 2'b11;
    #1;
    if (y !== 2'b00) begin
      $display("FAILED: ~(2'b11 & 2'b11) should be 2'b00, got %b", y);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule

module manyNands(y, a, b);
  parameter count = 3;
  output [count-1:0] y;
  input [count-1:0] a, b;

  assign y = ~(a & b);
endmodule
