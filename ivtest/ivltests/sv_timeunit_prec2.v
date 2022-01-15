timeunit 10us / 10us;

module fast_g (out);
   output out;
   reg	  out;

   initial begin
      #0 out = 0;
      #1 out = 1; // 10us
   end

endmodule // fast_g

`timescale 100us / 1us

// These will be ignored since a `timescale was already given.
timeunit 10us/10us;

module slow (out);
   output out;
   reg	  out;

   initial begin
      #0 out = 0;
      #1 out = 1; // 100us
   end

endmodule // slow


module fast (out);
   timeunit 10us/1us;
   output out;
   reg	  out;

   initial begin
      #0 out = 0;
      #1 out = 1; // 10us
   end

endmodule // fast


module saf(out);
   output out;
   reg	  out;

   initial begin
      #0 out = 0;
      #1 out = 1; // 100us
   end

endmodule // saf

`timescale 1us / 1us
module main;
   reg pass;
   wire slow, fast, fast_g, saf;

   slow m1 (slow);
   fast_g m2 (fast_g);
   fast m3 (fast);
   saf m4 (saf);

   initial begin
      pass = 1'b1;
      #9;
	if (slow !== 1'b0) begin
	   $display("FAILED: slow at 9us, expected 1'b0, got %b.", slow);
	   pass = 1'b0;
	end

	if (saf !== 1'b0) begin
	   $display("FAILED: saf at 9us, expected 1'b0, got %b.", saf);
	   pass = 1'b0;
	end

        if (fast !== 1'b0) begin
	   $display("FAILED: fast at 9us, expected 1'b0, got %b.", fast);
	   pass = 1'b0;
	end

        if (fast_g !== 1'b0) begin
	   $display("FAILED: fast_g at 9us, expected 1'b0, got %b.", fast_g);
	   pass = 1'b0;
	end

      #2 // 11us
	if (slow !== 1'b0) begin
	   $display("FAILED: slow at 11us, expected 1'b0, got %b.", slow);
	   pass = 1'b0;
	end

	if (saf !== 1'b0) begin
	   $display("FAILED: saf at 11us, expected 1'b0, got %b.", saf);
	   pass = 1'b0;
	end

        if (fast !== 1'b1) begin
	   $display("FAILED: fast at 11us, expected 1'b1, got %b.", fast);
	   pass = 1'b0;
	end

        if (fast_g !== 1'b1) begin
	   $display("FAILED: fast_g at 11us, expected 1'b1, got %b.", fast_g);
	   pass = 1'b0;
	end

      #88 // 99 us
	if (slow !== 1'b0) begin
	   $display("FAILED: slow at 99us, expected 1'b0, got %b.", slow);
	   pass = 1'b0;
	end

	if (saf !== 1'b0) begin
	   $display("FAILED: saf at 99us, expected 1'b0, got %b.", saf);
	   pass = 1'b0;
	end

        if (fast !== 1'b1) begin
	   $display("FAILED: fast at 99us, expected 1'b1, got %b.", fast);
	   pass = 1'b0;
	end

        if (fast_g !== 1'b1) begin
	   $display("FAILED: fast_g at 99us, expected 1'b1, got %b.", fast_g);
	   pass = 1'b0;
	end

      #2 // 101 us
	if (slow !== 1'b1) begin
	   $display("FAILED: slow at 101us, expected 1'b1, got %b.", slow);
	   pass = 1'b0;
	end

	if (saf !== 1'b1) begin
	   $display("FAILED: saf at 101us, expected 1'b1, got %b.", saf);
	   pass = 1'b0;
	end

        if (fast !== 1'b1) begin
	   $display("FAILED: fast at 101us, expected 1'b1, got %b.", fast);
	   pass = 1'b0;
	end

        if (fast_g !== 1'b1) begin
	   $display("FAILED: fast_g at 101us, expected 1'b1, got %b.", fast_g);
	   pass = 1'b0;
	end

      if (pass) $display("PASSED");

   end // initial begin
endmodule // main
