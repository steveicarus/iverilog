module test ();
   generate
      if (1) begin
         initial begin : a
            integer i;
            i=0;
	    $display("PASSED");
         end
      end
   endgenerate
endmodule
