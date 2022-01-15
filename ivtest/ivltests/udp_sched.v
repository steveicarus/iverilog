/*
 * This test tries to assure that all synchronous UDP outputs are
 * scheduled before any non-blocking assignment event. The reason
 * is that primitive outputs are scheduled in the active event
 * queue, which is supposed to empty before any non-blocking
 * assignments take effect.
 *
 * This is based on an example by Steve Sharp
 */

primitive u_dff(q,d,c);
output q;
reg q;
input d,c;
table
//d c : q : q+
  0 p : ? : 0 ;
  1 p : ? : 1 ;
  ? n : ? : - ;
  * ? : ? : - ;
endtable
endprimitive


module top;

reg rclk, dclk;
wire clk = rclk;
wire q0,q1,q2,q3,q4;

u_dff ff0(q0, 1'b1, clk),
      ff1(q1, 1'b1, q0),
      ff2(q2, 1'b1, q1),
      ff3(q3, 1'b1, q2),
      ff4(q4, 1'b1, q3);

initial
begin
	#1
	// Blocking assign makes an active event that
	// starts the u_dff devices rippling
	rclk = 1;

        // Non-blocking assign and the following @(dclk) pause
        // the thread until the non-blocking event queue is
        // processed.
	dclk <= 1;
	@(dclk)

	if (q4 !== 1'b1) begin
	   $display("FAILED -- q4 did not propagate in time (q4=%b)", q4);
	   $finish;
	end

	$display("q4=%b", q4);
        $display("PASSED");
end

endmodule
