primitive UDP (Q, CK, DAT, CL);
output Q;
input CK, DAT, CL;
reg Q;
    table
    // CK DAT CL  : Q :Q+;
       r   0   1  : ? : 0;
       r   1   1  : ? : 1;
       f   ?   1  : ? : -;
       b   ?   0  : ? : 0;
       ?   ?  (bx): ? : x;
    endtable
endprimitive

module flop (Q, CK, DAT, CL);
output Q;
input CK, DAT, CL;

    UDP udp (Q, CK, DAT, CL);

endmodule

module test;
    reg CL, D, CK, foo;
    wire Q;

    initial begin
	$monitor ($time,, "CL = %b, D = %b, Q = %b", CL, D, Q);
	CL = 1'b0;
	D = 1'b1;
	CK = 1'b0;
	#10;
	CL = 1'bx;
	#10;
	CL = 1'b0;
	#10;
	CL = 1'bx;
	#10;
	CL = 1'b1;
	repeat (3) @(negedge CK);
	$finish(0);
    end
    always #10 CK = !CK;

    flop ff (Q, CK, D, CL);

endmodule
