

module test
  (output reg [1:0] foo,
   input wire [1:0] addr,
   input wire	    in0, in1,
   input wire	    en0, en1
   /* */);

   localparam foo_default = 2'b00;
   always @*
     begin
	foo = foo_default;
	case (addr)
	  0: if (en0) foo[0] = in0;
	  1: if (en1) foo[1] = in1;
	  2: foo = {in1, in0};
	  default: foo = 0;
	endcase
     end

endmodule // test

module main;

   wire [1:0] foo;
   reg [1:0]  addr;
   reg	      in0, in1;
   reg	      en0, en1;

   test dut(.foo(foo), .addr(addr), .in0(in0), .in1(in1), .en0(en0), .en1(en1));

   initial begin
      in0 = 1;
      in1 = 1;
      en0 = 1;
      en1 = 1;

      addr = 3;
      #1 if (foo !== 2'b00) begin
	 $display("FAILED -- foo=%b, in=%b%b, en=%b%b, addr=%b",
		  foo, in1, in0, en1, en0, addr);
	 $finish;
      end

      addr = 0;
      #1 if (foo !== 2'b01) begin
	 $display("FAILED -- foo=%b, in=%b%b, addr=%b", foo, in1, in0, addr);
	 $finish;
      end

      addr = 1;
      #1 if (foo !== 2'b10) begin
	 $display("FAILED -- foo=%b, in=%b%b, addr=%b", foo, in1, in0, addr);
	 $finish;
      end

      addr = 2;
      #1 if (foo !== 2'b11) begin
	 $display("FAILED -- foo=%b, in=%b%b, addr=%b", foo, in1, in0, addr);
	 $finish;
      end

      en0 = 0;
      en1 = 0;

      addr = 3;
      #1 if (foo !== 2'b00) begin
	 $display("FAILED -- foo=%b, in=%b%b, en=%b%b, addr=%b",
		  foo, in1, in0, en1, en0, addr);
	 $finish;
      end

      addr = 0;
      #1 if (foo !== 2'b00) begin
	 $display("FAILED -- foo=%b, in=%b%b, en=%b%b, addr=%b",
		  foo, in1, in0, en1, en0, addr);
	 $finish;
      end

      addr = 1;
      #1 if (foo !== 2'b00) begin
	 $display("FAILED -- foo=%b, in=%b%b, addr=%b", foo, in1, in0, addr);
	 $finish;
      end

      addr = 2;
      #1 if (foo !== 2'b11) begin
	 $display("FAILED -- foo=%b, in=%b%b, en=%b%b, addr=%b",
		  foo, in1, in0, en1, en0, addr);
	 $finish;
      end

      $display("PASSED");
   end

endmodule // main
