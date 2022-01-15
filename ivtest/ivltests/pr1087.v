//**************************************************************************
module COUNTER(CLOCK_I, nARST_I, COUNT_O);

parameter CBITS = 3;

input	CLOCK_I;
input	nARST_I;

output[CBITS-1:0]	COUNT_O;

reg[CBITS-1:0]	COUNT_O;

always @(posedge CLOCK_I or negedge nARST_I)
  if(nARST_I==1'b0)
    COUNT_O	<= {CBITS{1'b0}};
  else
    COUNT_O	<= COUNT_O + 1;

endmodule

//--------------------------------------------------------------------------

module MULTIPLE_COUNTERS(CLOCK_I, nARST_I, COUNT_O);

parameter M = 3;
parameter CBITS = 4;

input	CLOCK_I;
input	nARST_I;

output[M*CBITS-1:0]	COUNT_O;

COUNTER #(.CBITS(CBITS)) INST_COUNTER[M-1:0] (CLOCK_I, nARST_I, COUNT_O);

endmodule

//--------------------------------------------------------------------------

module TEST_COUNTER;

parameter M = 2;
parameter CBITS = 2;

reg	CLOCK;
reg	nARST;
reg	CTRL_I;

wire[M*CBITS-1:0]	COUNTS;

MULTIPLE_COUNTERS #(.M(M),
		    .CBITS(CBITS)) INST_MCTR(CLOCK, nARST, COUNTS);

initial CLOCK=1;
always #5 CLOCK=~CLOCK;

initial
  begin
  nARST=1;
  #5 nARST=0;
  #5 nARST=1;
  #200 $display("PASSED");
  $finish;
  end


endmodule
