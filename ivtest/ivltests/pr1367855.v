// test.v program starts here
// This program is based on iverilog report [ 1367855 ] vvp simulation error
module test();
  reg [3:0] S;

  mux m( .SEL(S) );

  initial begin
   S=3; #100;
   S=2; #100;
   $display("PASSED");
   $finish;
  end
endmodule

module mux(SEL);
input [3:0] SEL;
wire [3:0] SEL;
integer offset;

always @(SEL) begin
   offset = SEL[3] + SEL[0]*128 + SEL[2:1]*2;
   $display("MUX: SEL=%d offset=%b", SEL, offset);

   case (SEL)

     'bxxxx: begin
     end

     2: if (offset !== 'b00000000000000000000000000000010) begin
	$display("FAILED");
	$finish;
     end

     3: if (offset !== 'b00000000000000000000000010000010) begin
	$display("FAILED");
	$finish;
     end

     default: begin
	$display("FAILED -- SEL=%b", SEL);
	$finish;
     end

   endcase
end
endmodule
