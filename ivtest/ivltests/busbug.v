module busbug ();
   reg        x,y,z;
   initial begin
      x=1'b0;
      y=1'b0;
      z=1'b1;
      $display("%b%b=%b",
                x ^ y, x ^ (y ^ z),
               {x ^ y, x ^ (y ^ z)}
               );
   end
endmodule
