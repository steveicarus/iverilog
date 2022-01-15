module test;
   initial
     begin: A1
	while(1 != 0)
	  begin: A2
	     reg B;
	     B = 1;
	     $display("B is %d", B);
	     disable A1;
	  end
     end
endmodule
