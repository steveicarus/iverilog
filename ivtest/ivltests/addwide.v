// (c) Alex.Perry@qm.com - 2002, Quantum Magnetics Inc, San Diego CA
// This source file is licensed under the GNU public license version 2.0
// All other rights reserved.

// NOTE: This test catches addition of wide (>16 bits) constants
// to wide vectors. -- Steve Williams
module source ( C, h );
output [ 0:0] C;
output [11:0] h;
reg    [ 0:0] C;
reg    [11:0] h;
reg    [21:0] l;

parameter kh = 3;
parameter kl = 21'd364066;
parameter wl = 21'h100000;

initial #5
	begin
	C <= 0;
	l <= 0;
	h <= 0;
	end
always #10 C = ~C;
always @(posedge C)
begin	if ( l >= wl )
	begin	l <= l + kl - wl;
		h <= h + kh + 1;
	end else
	begin	l <= l + kl;
		h <= h + kh;
	end
end

endmodule


module bench;
wire [ 0:0] clock;
wire [11:0] h;
source dut ( .C(clock), .h(h) );
initial #85
	begin
	if ( h == 13 ) begin
		$display ( "%7d", h );
		$display ("PASSED");
	end else begin
		$display ( "%7d = FAIL", h );
	end
	$finish;
	end

endmodule
