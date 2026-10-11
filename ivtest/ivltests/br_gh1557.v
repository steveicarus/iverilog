/* sv_class_packed_bitsel.v — regression for GitHub issue #1557
 *
 * Bit-select of a scalar packed class property must honour the index.
 * IEEE 1800-2023 §11.5.1: c.vect[2] with vect=8'h04 (00000100) → 1.
 *
 * Before the fix in elab_expr.cc the index was silently dropped:
 *   - assignment path returned bit 0 instead of bit 2
 *   - $display path returned all 8 bits instead of 1 bit
 */

class C;
   logic [7:0] vect;
endclass

module top;
   C c;
   logic b;

   initial begin
      c = new;

      /* ---- constant index ---- */
      c.vect = 8'h04;   // 00000100: bit 2 = 1, bit 0 = 0
      b = c.vect[2];
      $display("c.vect[2]=%b (expect 1)", b);
      if (b !== 1'b1) begin
	 $display("FAILED: assign path c.vect[2]=%b (expect 1)", b);
	 $finish;
      end

      $display("c.vect[2] direct=%b (expect 1)", c.vect[2]);
      if (c.vect[2] !== 1'b1) begin
	 $display("FAILED: direct c.vect[2]=%b (expect 1)", c.vect[2]);
	 $finish;
      end

      /* ---- make sure it is really bit 2, not bit 0 by coincidence ---- */
      c.vect = 8'hFB;   // 11111011: bit 2 = 0, bit 0 = 1
      b = c.vect[2];
      $display("c.vect[2]=%b (expect 0 for vect=8'hFB)", b);
      if (b !== 1'b0) begin
	 $display("FAILED: assign path c.vect[2]=%b (expect 0 for vect=8'hFB)", b);
	 $finish;
      end

      $display("c.vect[2] direct=%b (expect 0 for vect=8'hFB)", c.vect[2]);
      if (c.vect[2] !== 1'b0) begin
	 $display("FAILED: direct c.vect[2]=%b (expect 0 for vect=8'hFB)", c.vect[2]);
	 $finish;
      end

      /* ---- variable index ---- */
      c.vect = 8'h04;
      begin
	 integer i;
	 i = 2;
	 b = c.vect[i];
	 $display("c.vect[i=%0d]=%b (expect 1)", i, b);
	 if (b !== 1'b1) begin
	    $display("FAILED: variable index c.vect[i]=%b (expect 1)", b);
	    $finish;
	 end
      end

      $display("PASSED");
   end
endmodule
