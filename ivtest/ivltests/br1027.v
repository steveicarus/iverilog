module test;
   reg[2:0] a, b, c, d;

   initial begin
      $monitor($time, " a=%b; b=%b; c=%b; d=%b", a, b, c, d);
      a = 1;
      b = 2;
      c = 3;
      d = 4;
      #1;
      swap(a, b);
      #1;
      swap(c, d);
      #1;
      if (a === 2 && b ===1 && c === 4 && d === 3)
         $display("PASSED");
      else
         $display("FAILED");
   end

   task swap(inout reg[2:0] x, y);
      reg[2:0] temp;
      begin
         temp = x;
         x = y;
         y = temp;
//       $display("temp: %b; x: %b; y: %b", temp, x, y);
      end
   endtask
endmodule // test
