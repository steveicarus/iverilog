/*
 * 23.2.2.3 Rules for determining port kind, data type, and direction says
 *
 * For output ports, the default port kind depends on how the data type is
 * specified:
 * - If the data type is omitted or declared with the implicit_data_type
 *   syntax, the port kind shall default to a net of default net type.
 * - If the data type is declared with the explicit data_type syntax, the port
 *   kind shall default to variable.
 */

typedef enum { A, B } E;

module main;
   E in;
   wire E out;

   M foo (in, out);

   initial begin
      in = A;
      #1 if (out !== A) begin
	 $display("FAIL: in=%0d, out=%0d", in, out);
	 $finish;
      end

      in = B;
      #1 if (out !== B) begin
	 $display("FAIL: in=%0d, out=%0d", in, out);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end
endmodule // main

module M (input  E ei,
	  output E eo);

   always_comb eo = ei;

endmodule // M
