`timescale 1ns/1ns

`define DAC_MSB 7
`define ADC_MSB 15

`define NSEC 1
`define USEC (`NSEC*1000)
`define MSEC (`USEC*1000)


// TOPLEVEL TO STIMULATE
module toy_toplevel(
	        input  wire [`ADC_MSB:0]	V_load_adc,
	        input  wire			V_load_valid,
	        output reg			pwm,
	        output reg  [`DAC_MSB:0]	V_src
	    ) ;

	parameter time STARTUP_DELAY = 2 * `MSEC;

	parameter real ADC_RANGE = 32.0;
	parameter real ADC_OFFSET = -ADC_RANGE/2.0;
	parameter real DAC_RANGE = 16.0;
	parameter real DAC_OFFSET = -DAC_RANGE/2.0;
	parameter real UPDATE_FREQ_MHZ = 1.0;
	parameter time CLOCK_INTERVAL = `USEC / UPDATE_FREQ_MHZ;

	reg clk = 0;
	reg ls_only = 0;
	real V_load = 0.0;

	function real decode_value( input real base, input real range, input integer msb, input integer value );
	begin
		decode_value = base + range * value / $itor(1<< (msb+1));
	end
	endfunction

	function integer encode_value( input real base, input real range, input integer msb, input real value );
	begin
		encode_value = (value -base) * $itor(1<< (msb+1)) / range;
	end
	endfunction

	always @( posedge(V_load_valid) )
	begin
		V_load = decode_value( ADC_OFFSET, ADC_RANGE, `ADC_MSB, V_load_adc );
	end

	initial
	begin
		clk = 0;
		ls_only = 0;
		#( `USEC * 1 );
		# ( CLOCK_INTERVAL/4 );
		$finish(0); // Stop things for VPI unit test...
		forever
		begin
			# ( CLOCK_INTERVAL/2 );
			clk <= ! clk;
		end

	end

	always @clk
	begin
		ls_only= (V_load >2.5);
		pwm <= clk | ls_only;
	end

	initial
	begin
	    V_src = encode_value( DAC_OFFSET, DAC_RANGE, `DAC_MSB, 7.2 );
	end

endmodule
