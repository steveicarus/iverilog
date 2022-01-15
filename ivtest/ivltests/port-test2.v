// This is a compile time test,
// for various port declaration syntax options

`define TEST3
// `define TEST3_X

// unconnected ports

`ifdef TEST3
module port_3
  (
    dummy_1,
   /* unconnected */,
    in[7:0],
    dummy_2,
    out[7:0],
   /* unconnected */
   );
   input [7:0]  in;
   output [7:0] out;
   output	dummy_1;
   output	dummy_2;
   assign	out = in;
endmodule
`endif // ifdef TEST_3


module port_test;

   reg [7:0] data;

`ifdef TEST3
   wire [7:0] out_3;
   reg	      pass_3;
   initial    pass_3 = 1;
   port_3 dut_3
     (,          // unconnected dummy_1
`ifdef TEST3_X
// This fails in verilog-XL with:
// Error!    Expression given for a null module port           [Verilog-EXPNMP]
//           "port-test.v", 115: dut_3(, pass_3, data[7:0], ,
//           out_3[7:0], )
      pass_3,    // dummy       unconnected
`else
      ,          // unconnected unconnected
`endif
      data[7:0],
      ,          // unconnected dummy_2
      out_3[7:0],
                 // unconnected unconnected
      );
`endif

   initial
     begin
	data <= 1;
	#1;

	while (data != 0)
	  begin
	     $display ("%b", data);

`ifdef TEST3
	     if (out_3 != data)
	       begin
		  $display("data=%b, out_2=%b, FAILED", data, out_3);
		  pass_3 = 0;
	       end
`endif

	     data <= data << 1;
	     #1;
	  end

`ifdef TEST3
	if (pass_3)
	  $display("PASSED");
`endif

	$finish;
     end

endmodule // port_test
