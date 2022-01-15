//
// Test for PR#707, NULL UDP port connections
//
primitive mux (x, s, a, b, f);
output x;
input s, a, b, f;
table
 // s a b f  mux
    0 1 ? ? : 1 ; // ? = 0 1 x
    0 0 ? ? : 0 ;
    1 ? 1 ? : 1 ;
    1 ? 0 ? : 0 ;
    x 0 0 ? : 0 ;
    x 1 1 ? : 1 ;
endtable
endprimitive

module test;
    reg r1, r2, r3;
    wire w1;

    initial begin
	r1 = 1'b0;
	r2 = 1'b0;
	r3 = 1'b0;
	// If it makes it here, the code compiled
	$display("PASSED");
    end
    mux udp1(w1, r1, r2, r3, /* foo */);
endmodule
