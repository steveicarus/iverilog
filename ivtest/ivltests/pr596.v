`timescale 1ns/1ns

module lfsr_test();
  parameter SIZE = 4;
  reg clk, reset, ena;
  wire [SIZE-1:0] out;

  initial
    begin //{
      clk = 0;
      reset = 0;
      ena = 1'bz;
      #15 reset = 0;
      #20 reset = 1;
    end //}

   initial
      begin //{
	 //$dumpfile("lfsr_test.vcd"); // Change filename as appropriate.
	 //$dumpvars( 0, lfsr_test);
	 $monitor("out=%b", out);
      end //}

  always clk = #10 ~clk;

  lfsr_counter LF( clk, reset, out );

  initial #1000 $finish(0);
endmodule // gray_code




module lfsr_counter( clk, reset, out );
  `define W 4
  parameter WIDTH = `W;
  parameter TAP = `W'b1001;
  integer   N;

  output [WIDTH-1:0] out;
  input              clk, reset;

  wire [WIDTH-1:0] gc;
  reg [WIDTH-1:0]  lfsr, next_lfsr;
  reg		   fb_lsb, fb;

  always @(posedge clk or negedge reset )
    begin //{
      if( reset == 1'b0 )
	lfsr[WIDTH-1:0] <= `W'b0;
      else
	lfsr[WIDTH-1:0] <= next_lfsr[WIDTH-1:0];
    end //}

  always @( lfsr )
    begin //{
      fb_lsb = ~| lfsr[WIDTH-2:0];
      fb = lfsr[WIDTH-1] ^ fb_lsb;
      for( N=WIDTH; N>=0; N=N-1 )
	if( TAP[N] == 1 )
	  next_lfsr[N] = lfsr[N-1] ^ fb;
	else
	  next_lfsr[N] = lfsr[N-1];
      next_lfsr[0] = fb;
      end //}
  assign out[WIDTH-1:0] = {1'b0, lfsr[WIDTH-1:1]};  //(1)
  //assign out[WIDTH-1:0] = lfsr[WIDTH-1:0];  //(2)
  //assign gc[WIDTH-1:0] = out[WIDTH-1:0] ^ {1'b0, out[WIDTH-1:1]};
//(3)

endmodule // gray_counter
