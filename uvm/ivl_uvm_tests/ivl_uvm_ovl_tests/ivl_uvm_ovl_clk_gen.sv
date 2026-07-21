// `timescale 1ns/1fs

module ivl_uvm_ovl_clk_gen 
  #(parameter real FREQ_IN_MHZ = 11.11,
    parameter int DUTY_CYCLE = 50 // In percentage as int 
	 )
	(output logic clk);
	timeunit 1ns;
	timeprecision 1fs;
  
  real clk_period  		= ( (1000.0/(FREQ_IN_MHZ))  ); // convert to ns
  real clk_on_period  		= DUTY_CYCLE/100.0 * clk_period;
  real clk_off_period 		= (100.0 - DUTY_CYCLE)/100.0 * clk_period;
 
	bit clk_en = 1;  
  
  initial begin    
    $display("%m FREQ_IN_MHZ = %0.3f MHz", FREQ_IN_MHZ);
    $display("%m PERIOD    = %0.3f ns", clk_period);    
  end
  
	always begin : cgen
    clk <= 0;
      
    while (clk_en) begin
      #(clk_on_period)  clk = 0;
    	#(clk_off_period) clk = 1;
    end
  end : cgen 

endmodule : ivl_uvm_ovl_clk_gen 

