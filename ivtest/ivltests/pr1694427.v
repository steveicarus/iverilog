module test ();

   reg[7:0]   a;
   reg        b;

   always @*
     begin
	b = 1'b0;
	case (a)
	  8'd66: b = 1'b1;
	  default:		        ;
	endcase
     end

   initial begin
      a = 0;
      #1 if (b !== 0) begin
	 $display("FAILED  -- a=%h  b=%b", a, b);
	 $finish;
      end

      a = 66;
      #1 if (b !== 1) begin
	 $display("FAILED  -- a=%h  b=%b", a, b);
	 $finish;
      end

      $display("PASSED");
   end
endmodule
