`timescale 1ns/1ps
module test;
   reg in, pass;
   wire out;

   assign #(1?2:1) out = in;
//   assign #(1+1) out = in;

  initial begin
    pass = 1'b1;
    in = 1'b0;
    #1.999;
    if (out !== 1'bx) begin
      $display("Failed signal at begining, expected 1'bx, got %b", out);
      pass = 1'b0;
    end
    #0.002;
    if (out !== in) begin
      $display("Failed signal at end, expected %b, got %b", in, out);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
