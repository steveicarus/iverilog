/*
 * Passing a whole unpacked array to a function or task, and copying one
 * unpacked array to another at run time. Covers ascending and descending
 * declared ranges, two dimensions, real elements, and x propagation.
 */
module top;

   logic [7:0] asc [0:3];
   logic [7:0] cpy [0:3];
   logic [7:0] dsc [3:0];
   logic [3:0] m2d [0:1][0:2];
   logic [3:0] n2d [0:1][0:2];
   real        rr  [0:3];
   real        rc  [0:3];

   int errors = 0;

   function automatic logic [7:0] sum4(input logic [7:0] a [0:3]);
      logic [7:0] r;
      begin
	 r = '0;
	 for (int i = 0; i < 4; i++) r += a[i];
	 return r;
      end
   endfunction

   function automatic logic [7:0] sum4d(input logic [7:0] a [3:0]);
      logic [7:0] r;
      begin
	 r = '0;
	 for (int i = 0; i < 4; i++) r += a[i];
	 return r;
      end
   endfunction

   function automatic real rsum(input real a [0:3]);
      real r;
      begin
	 r = 0.0;
	 for (int i = 0; i < 4; i++) r = r + a[i];
	 return r;
      end
   endfunction

   task automatic copy2d(input logic [3:0] s [0:1][0:2]);
      n2d = s;
   endtask

   // A function used from a procedural context, taking an array argument.
   logic [7:0] comb_out;
   always_comb comb_out = sum4(asc);

   initial begin
      for (int i = 0; i < 4; i++) asc[i] = 8'(i + 1);
      for (int i = 0; i < 4; i++) dsc[i] = 8'(i + 1);
      for (int i = 0; i < 2; i++)
	for (int j = 0; j < 3; j++) m2d[i][j] = 4'(i*3 + j);
      for (int i = 0; i < 4; i++) rr[i] = i * 1.5;

      // whole unpacked array assignment at run time
      cpy = asc;
      if (cpy[0] !== 8'd1 || cpy[3] !== 8'd4) begin
	 $display("FAILED -- array copy cpy[0]=%0d cpy[3]=%0d", cpy[0], cpy[3]);
	 errors = errors + 1;
      end

      // array passed to a function, ascending and descending ranges
      if (sum4(asc) !== 8'd10) begin
	 $display("FAILED -- sum4=%0d", sum4(asc));
	 errors = errors + 1;
      end
      if (sum4d(dsc) !== 8'd10) begin
	 $display("FAILED -- sum4d=%0d", sum4d(dsc));
	 errors = errors + 1;
      end

      // array passed to a task, two dimensions
      copy2d(m2d);
      if (n2d[1][2] !== 4'd5) begin
	 $display("FAILED -- copy2d n2d[1][2]=%0d", n2d[1][2]);
	 errors = errors + 1;
      end

      // real elements, both copy and argument
      rc = rr;
      if (rc[3] != 4.5) begin
	 $display("FAILED -- real copy rc[3]=%f", rc[3]);
	 errors = errors + 1;
      end
      if (rsum(rr) != 9.0) begin
	 $display("FAILED -- rsum=%f", rsum(rr));
	 errors = errors + 1;
      end

      // x propagates through a copy
      asc[2] = 8'bxxxxxxxx;
      cpy = asc;
      if (cpy[2] !== 8'bxxxxxxxx) begin
	 $display("FAILED -- x propagation cpy[2]=%b", cpy[2]);
	 errors = errors + 1;
      end

      // the always_comb above should have tracked asc
      #1;
      if (comb_out !== sum4(asc)) begin
	 $display("FAILED -- always_comb out=%0d expected %0d", comb_out, sum4(asc));
	 errors = errors + 1;
      end

      if (errors == 0) $display("PASSED");
      $finish;
   end

endmodule
