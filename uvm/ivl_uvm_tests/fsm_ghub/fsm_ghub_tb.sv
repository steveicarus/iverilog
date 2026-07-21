`timescale 1ns/1ns

module fsm_tb;

  import ivl_uvm_pkg::*;

wire [1:0]Z; //Taking outputs as wire
reg A; // Taking inputs as reg
reg clock,reset;

// Clock signal is defined in the block below
always begin 
	clock = 1; 
	#10; 
	clock = 0; 
	#10; 
end 

// Instantiating the topmodule in the test bench to try simulations over it
fsm top(.reset(reset),.clock(clock),.A(A),.Z(Z));

initial begin 
	//$dumpfile is used to dump the value changes of nets and registers in a file that is named as its argument. 
	$dumpfile("fsm_tb.vcd"); 

	//Above statement will dump the changes in a file named fsm_tb.vcd (value change dump) which stores all information about value change but, what we are exactly going to dump in this file is told by $dumpvars

	$dumpvars(0,fsm_tb); 
	/* 
	When level is set to 0, and only the module name is specified, it dumps all the variables of that module and all the variables in ALL lower level modules instantiated by this module. 
		If any module in not instantiated by this top module, then its variable will not be 
		covered. 
			*/ 

		 A = 0;
		 reset = 1;
		 #20;
		 reset = 0;
		 #20;
		 A = 1;
		 #100;
		 A = 0;
		 #100;
		 A = 1;
		 #20;
		 reset = 1;
		 #20;
		 $finish;
	 end

	 always @(*) begin
		 `g2u_printf (("reset: %b A: %b", reset, A))
	 end

	 endmodule

