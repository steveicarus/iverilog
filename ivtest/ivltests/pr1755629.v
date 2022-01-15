module test;

   parameter       some = 4;
   wire [some-1:0] flag1;

   genvar i;

   generate
      for (i = 0; i < some; i = i + 1)
	begin : what
	   wire [some-1:0] slice;
	end
   endgenerate

   generate
      for (i = 0; i < some; i = i + 1)
	begin : ab
	   assign what[i].slice[i] = 1'b1;
	   assign flag1[i] = &what[i].slice;
	end
   endgenerate

   integer	  idx;
   initial #1 begin
      for (idx = 0 ; idx < some ;  idx = idx+1) begin
	 if (flag1[idx] !== 1'bx) begin
	    $display("FAILED -- flag1=%b", flag1);
	    $finish;
	    end
      end
      $display("PASSED");
      $finish;
   end

endmodule
