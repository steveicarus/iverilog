module main;

   reg [7:0] period;

   reg	     drive;
   wire      trace;

   // This is the main point of the test. Non-constant delay expressions
   // should work here.
   assign #(period) trace = drive;

   initial begin
      period = 8;
      // Initially, set up a period=8 and get the trace to start
      // following the drive.
      #1 drive <= 1;
      #9 if (trace !== drive) begin
	 $display("FAILED -- time=%0t, drive=%b, trace=%b",
		  $time, drive, trace);
	 $finish;
      end

      // The drive should NOT change the trace before the period.
      drive <= 0;

      #7 if (trace !== 1'b1) begin
	 $display("FAILED -- time=%0t, drive=%b, trace=%b",
		  $time, drive, trace);
	 $finish;
      end
      #2 if (trace !== drive) begin
	 $display("FAILED -- time=%0t, drive=%b, trace=%b",
		  $time, drive, trace);
	 $finish;
      end

      // Change the period.
      period = 6;

      // Now check that the new delay is taken.
      #1 drive <= 1;

      #5 if (trace !== 1'b0) begin
	 $display("FAILED -- time=%0t, drive=%b, trace=%b",
		  $time, drive, trace);
	 $finish;
      end

      #2 if (trace !== drive) begin
	 $display("FAILED -- time=%0t, drive=%b, trace=%b",
		  $time, drive, trace);
	 $finish;
      end

      $display("PASSED");
      $finish;
   end

endmodule // main
