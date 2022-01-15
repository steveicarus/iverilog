timeunit 1ns;
timeprecision 10ps;

module test;

parameter factor = 1e-9/10e-12;

longint tmanual, tnow, tdiff;
longint incr;

initial begin
    tmanual = 0;
    if ($realtime != 0) begin
		$display ("FAILED");
		$finish;
	end
	#33.1ns;
	incr = 33.1e-9/10e-12;
	tmanual = tmanual + incr;
	tnow  = $realtime*factor;
	tdiff = tnow-tmanual;
    if (tdiff != 0) begin
		$display ("FAILED");
		$finish;
	end
	#78.1ps;
	incr = 78.1e-12/10e-12;
	tmanual = tmanual + incr;
	tnow  = $realtime*factor;
	tdiff = tnow-tmanual;
    if (tdiff != 0) begin
		$display ("FAILED");
		$finish;
	end
	#123.08ns;
	incr = 123.08e-9/10e-12;
	tmanual = tmanual + incr;
	tnow  = $realtime*factor;
	tdiff = tnow-tmanual;
    if (tdiff != 0) begin
		$display ("FAILED");
		$finish;
	end
	#9.006ns;
	incr = 9.006e-9/10e-12;
	tmanual = tmanual + incr;
	tnow  = $realtime*factor;
	tdiff = tnow-tmanual;
    if (tdiff != 0) begin
		$display ("FAILED");
		$finish;
	end
	#17.003ns;
	incr = 17.003e-9/10e-12;
	tmanual = tmanual + incr;
	tnow  = $realtime*factor;
	tdiff = tnow-tmanual;
    if (tdiff != 0) begin
		$display ("FAILED");
		$finish;
	end
	#578.23us;
	incr = 578.23e-6/10e-12;
	tmanual = tmanual + incr;
	tnow  = $realtime*factor;
	tdiff = tnow-tmanual;
    if (tdiff != 0) begin
		$display ("FAILED");
		$finish;
	end
	#0.0356ms;
	incr = 0.0356e-3/10e-12;
	tmanual = tmanual + incr;
	tnow  = $realtime*factor;
	tdiff = tnow-tmanual;
    if (tdiff != 0) begin
		$display ("FAILED");
		$finish;
	end
	#1.011s;
	incr = 1.011e0/10e-12;
	tmanual = tmanual + incr;
	tnow  = $realtime*factor;
	tdiff = tnow-tmanual;
    if (tdiff != 0) begin
		$display ("FAILED");
		$finish;
	end
	$display("PASSED");
	//$display ("Time now is: %t, manual = %0d, tnow = %0d, diff = %0d ", $realtime, tmanual, tnow, tdiff);
	$finish;
end

endmodule
