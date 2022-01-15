/*
 * Based on Request id 2091455 in the iverilog Bugs database
*/
module main;

   parameter foo = 2;

   generate
      case (foo)
	0: initial #1 $display("I am in %m, case foo=%0d", foo);
	1: initial #1 $display("I am in %m, case foo=%0d", foo);
	2: initial #1 $display("I am in %m, case foo=%0d", foo);
      endcase // case (foo)
      case (foo)
	0: begin : X initial $display("I am in %m, case foo=%0d", foo); end
	1: begin : X initial $display("I am in %m, case foo=%0d", foo); end
	2: begin : X initial $display("I am in %m, case foo=%0d", foo); end
      endcase // case (foo)
   endgenerate

endmodule // bug
