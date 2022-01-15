module test();
reg [13:0]     a;
reg            b;
reg c;

always @(a or b)
begin

   case ({1'b0,~b,a[3:0]})
      6'b00_0000          : begin
         c        = 1'b1;
      end
      default       : begin
         c        = 1'b0;
      end
   endcase
end

initial begin
   #1 /* Wait for the always block above to get settled. */;
   a = 0;
   b = 0;
   #1 if (c !== 0) begin
      $display("FAILED - a=%b, b=%b, c=%b", a, b, c);
      $finish;
   end

   b = 1;
   #1 if (c !== 1) begin
      $display("FAILED - a=%b, b=%b, c=%b", a, b, c);
      $finish;
   end

   $display("PASSED");
end // initial begin

endmodule
