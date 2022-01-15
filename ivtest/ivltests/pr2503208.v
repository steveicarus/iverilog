module tb;

parameter A = 34;
parameter B = 17;
parameter C = ((A+1)>>1) + (B-((A+1)>>1));

initial begin
   if (C !== 17) begin
      $display("FAILED -- C == %d (%b)", C, C);
      $finish;
   end
   $display("PASSED");
   $finish;
end

endmodule
