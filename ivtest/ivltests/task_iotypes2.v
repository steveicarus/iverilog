module main;

   task take_args;

      input integer iarg;
      input real rarg;
      output integer iout;
      output real rout;

      begin
	 iout = iarg + 1;
	 rout = rarg + 1.0;
      end

   endtask // take_args

   integer ii, io;
   real    ri, ro;

   initial begin
      ii = 4;
      ri = 6.0;
      io = 0;
      ro = 0.0;

      take_args(ii,ri,io,ro);

      if (io !== 5) begin
	 $display("FAILED -- ii=%d, io=%d", ii, io);
	 $finish;
      end

      if (ro != 7.0) begin
	 $display("FAILED -- ri=%f, ro=%f", ri, ro);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
