/*
 * Unpacked array parameters used in the ways reported against PR #501:
 *   - an element of one array parameter feeding the initialiser of another,
 *     nested two levels deep
 *   - an element inside a constant function call ($clog2)
 *   - `integer` (signed, 4-state) elements rather than logic vectors
 *   - a run-time variable index, in both a continuous assignment and a
 *     procedural block
 * Each of these crashed that branch with a different assertion.
 */
module top;

   localparam integer NTAPS  [0:3] = '{ 960, 540, 360, 180 };
   localparam integer R      [0:3] = '{ 20, 10, 4, 2 };

   // element of a previous array parameter, with arithmetic
   localparam integer NCOEFF [0:3] = '{ NTAPS[0]/2, NTAPS[1]/2,
					NTAPS[2]/2, NTAPS[3]/2 };

   // scalar parameter folded from array elements
   localparam integer TOTAL = NCOEFF[0] + NCOEFF[1] + NCOEFF[2] + NCOEFF[3];

   // array element inside a constant system function
   localparam integer N_PSF_W   = $clog2(R[0]);
   localparam integer N_PCOEF_W = $clog2(NCOEFF[3] / R[3]);

   // two levels of parameter referencing parameter
   localparam integer T2 [0:1] = '{ 32, 688 };
   localparam integer C2 [0:1] = '{ T2[0]/2, T2[1]/2 };
   localparam integer B2 [0:1] = '{ 0, C2[0] };

   reg  [1:0]         f;
   wire signed [15:0] y_ca;
   reg  signed [15:0] y_pr;

   assign y_ca = NTAPS[f];        // variable index, continuous assignment
   always @* y_pr = NTAPS[f];     // variable index, procedural

   integer errors = 0;

   task ck(input [8*16:1] nm, input integer got, input integer exp);
      begin
	 if (got !== exp) begin
	    $display("FAILED -- %0s got %0d expected %0d", nm, got, exp);
	    errors = errors + 1;
	 end
      end
   endtask

   initial begin
      ck("NCOEFF[0]", NCOEFF[0], 480);
      ck("NCOEFF[3]", NCOEFF[3], 90);
      ck("TOTAL",     TOTAL,     1020);
      ck("N_PSF_W",   N_PSF_W,   5);   // $clog2(20)
      ck("N_PCOEF_W", N_PCOEF_W, 6);   // $clog2(45)
      ck("B2[1]",     B2[1],     16);  // C2[0] = 32/2

      for (int i = 0; i < 4; i = i + 1) begin
	 f = i[1:0];
	 #1;
	 if (y_ca !== NTAPS[i]) begin
	    $display("FAILED -- continuous assign index %0d gave %0d", i, y_ca);
	    errors = errors + 1;
	 end
	 if (y_pr !== NTAPS[i]) begin
	    $display("FAILED -- procedural index %0d gave %0d", i, y_pr);
	    errors = errors + 1;
	 end
      end

      if (errors == 0) $display("PASSED");
      $finish;
   end

endmodule
