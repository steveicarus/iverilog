/***********************************************************************

  Duplicate output declaration test case
  Duplicate port declarations should generate an error

***********************************************************************/

module port_test4 (
		    a,		// Input
		    b,		// Output
                  );

input a;

output b;
output b;

assign b=a;

endmodule
