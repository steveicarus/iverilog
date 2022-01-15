module eqne(output wire eq1, output wire ne1,
	    output wire eq2, output wire ne2,
	    output wire eq5, output wire ne5,
	    input wire [7:0] x, input wire [7:0] y);

   assign	  eq1 = x[0] == y[0];
   assign	  ne1 = x[0] != y[0];
   assign	  eq2 = x[1:0] == y[1:0];
   assign	  ne2 = x[1:0] != y[1:0];
   assign	  eq5 = x[4:0] == y[4:0];
   assign	  ne5 = x[4:0] != y[4:0];

endmodule // eqne
