

module test
  (output reg [1:0] foo,
   input wire in0, en0,
   input wire in1, en1
   /* */);

   localparam foo_default = 2'b00;
   always @*
     begin
	foo = foo_default;
	if (en0) foo[0] = in0;
	if (en1) foo[1] = in1;
     end

endmodule // test

module main;

   wire [1:0] foo;
   reg	      in0, en0;
   reg	      in1, en1;

   test dut (.foo(foo), .in0(in0), .in1(in1), .en0(en0), .en1(en1));

   initial begin
      in0 = 1;
      in1 = 1;

      en0 = 0;
      en1 = 0;

      #1 if (foo !== 2'b00) begin
	 $display("FAILED -- foo=%b, in=%b%b, en=%b%b", foo, in1, in0, en1, en0);
	 $finish;
      end

      en0 = 1;
      #1 if (foo !== 2'b01) begin
	 $display("FAILED -- foo=%b, in=%b%b, en=%b%b", foo, in1, in0, en1, en0);
	 $finish;
      end

      en0 = 0;
      en1 = 1;
      #1 if (foo !== 2'b10) begin
	 $display("FAILED -- foo=%b, in=%b%b, en=%b%b", foo, in1, in0, en1, en0);
	 $finish;
      end

      en0 = 1;
      en1 = 1;
      #1 if (foo !== 2'b11) begin
	 $display("FAILED -- foo=%b, in=%b%b, en=%b%b", foo, in1, in0, en1, en0);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
