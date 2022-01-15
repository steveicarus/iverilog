`timescale 1us/100ns

module top;
  reg pass = 1;

  reg [3:0] ia = 4'd1, ib = 4'd2;
  wire [2:0] icon, irep;

  /* Integer concatenation. */
  assign #1 icon = {ib[1:0], ia[0]}; // 5

  /* Integer replication. */
  assign #1 irep = {3{ia[0]}}; // 7


  initial begin
    #0.9;
    if (icon !== 3'bx) begin
      pass = 1'b0;
      $display("Failed: concatenation is not delayed, expected 3'bx got %b.", icon);
    end
    if (irep !== 3'bx) begin
      pass = 1'b0;
      $display("Failed: replication is not delayed, expected 3'bx got %b.", irep);
    end
    #0.1;
    #0;
    if (icon !== 3'd5) begin
      pass = 1'b0;
      $display("Failed: concatenation has incorrect value, expected 3'd5 got %b.", icon);
    end
    if (irep !== 3'd7) begin
      pass = 1'b0;
      $display("Failed: replication has incorrect value, expected 3'd7 got %b.", irep);
    end
    if (pass) $display("PASSED");
  end
endmodule
