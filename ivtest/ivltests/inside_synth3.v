/*
 * This tests the latching of an output that isn't really an output,
 * but an intermediate symbol that is only used in some clauses.
 */
module main;

   reg [15:0] out, a;
   reg [7:0]  b;
   reg	      cy;
   reg	      with_carry;

   (* ivl_combinational *)
   always @(with_carry, a, b, cy)
     case (with_carry)
       1'b1:
	 begin
	    {cy, out[7:0]} = {1'b0, a[7:0]} + {1'b0, b[7:0]};
	    out[15:8] = a[15:8] + {7'b0, cy};
	 end
       1'b0:
	 begin
	    out = a + {8'h00, b};
	 end
     endcase

   (* ivl_synthesis_off *)
   initial begin
      a = 16'h00fe;
      b = 8'h00;
      with_carry = 0;
      #1 if (out !== 16'h00fe) begin
	 $display("FAILED -- a=%h, b=%h, out=%h", a, b, out);
	 $finish;
      end

      with_carry = 1;
      #1 if (out !== 16'h00fe) begin
	 $display("FAILED -- a=%h, b=%h, out=%h", a, b, out);
	 $finish;
      end

      b = 2;
      #1 if (out !== 16'h0100) begin
	 $display("FAILED -- a=%h, b=%h, out=%h", a, b, out);
	 $finish;
      end

      with_carry = 0;
      #1 if (out !== 16'h0100) begin
	 $display("FAILED -- a=%h, b=%h, out=%h", a, b, out);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
