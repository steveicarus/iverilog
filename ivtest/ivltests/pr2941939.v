`timescale 1ns/10ps

module top;
  reg pass;
  reg a;
  reg p;
  wire y;
  supply1 vdd;
  supply0 gnd;

  tranif1 #(5) nmos_0(gnd, y, a);
  tranif0 #(5) pmos_0(y, vdd, a);

  initial begin
    $monitor($realtime,, y,, a);
    pass = 1'b1;
    p = 1'bx;
    a <= 1'b0;
    repeat (2) #10 a = ~a;

    #10;
    if (pass) $display("PASSED");
    $finish;
  end

  always @(a) begin
    #4.99 if (y !== p) begin
      $display("Failed at %.2f (early), expected %b, got %b", $realtime, p, y);
      pass = 1'b0;
    end
    #0.02 if (y !== ~a) begin
      $display("Failed at %.2f (late), expected %b, got %b", $realtime, ~a, y);
      pass = 1'b0;
    end
    p = y;
  end
endmodule
