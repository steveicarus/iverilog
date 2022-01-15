module test ( a, b);

output        a;
output reg [31:0] b;

reg [1:0] c;

assign a = (b == {16{c}});

   initial begin
      c = 2'b01;
      b = 32'h55555555;
      #1 if (a !== 1) begin
	 $display("FAILED -- a=%b, b=%h, c=%b", a, b, c);
	 $finish;
      end

      b = 32'haaaaaaaa;
      #1 if (a !== 0) begin
	 $display("FAILED -- a=%b, b=%h, c=%b", a, b, c);
	 $finish;
      end

      $display("PASSED");
   end
endmodule
