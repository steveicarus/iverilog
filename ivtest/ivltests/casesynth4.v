
module test
  (output reg       a,
   output reg	    b,
   input wire [1:0] sel,
   input wire	    d
   /* */);

   always @* begin
      b = d;
      case (sel)
	0:
	  begin
	     a = 0;
	     b = 1;
	  end
	1:
	  begin
	     a = 1;
	     b = 0;
	  end
	default:
	  begin
	     a = d;
	  end
      endcase // case (sel)
   end // always @ *

endmodule // test

module main;
   reg [1:0] sel;
   reg	     d;
   wire      a, b;

   test dut (.a(a), .b(b), .sel(sel), .d(d));

   initial begin
      d = 0;
      sel = 0;
      #1 if (a!==0 || b!==1) begin
	 $display("FAILED -- sel=%b, d=%b, a=%b, b=%b", sel, d, a, b);
	 $finish;
      end
      sel = 1;
      #1 if (a!==1 || b!==0) begin
	 $display("FAILED -- sel=%b, d=%b, a=%b, b=%b", sel, d, a, b);
	 $finish;
      end
      sel = 2;
      #1 if (a!==0 || b!==0) begin
	 $display("FAILED -- sel=%b, d=%b, a=%b, b=%b", sel, d, a, b);
	 $finish;
      end
      d = 1;
      #1 if (a!==1 || b!==1) begin
	 $display("FAILED -- sel=%b, d=%b, a=%b, b=%b", sel, d, a, b);
	 $finish;
      end

      $display("PASSED");
   end
endmodule
