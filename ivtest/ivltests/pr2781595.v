module xortest(out, a, b);
    output out;
    input a, b;
    parameter tdelay=2;
    wire a_, b_, i1, i2, i3;

    supply0 gnd;
    supply1 vdd;

    nmos #(tdelay) n5(a_, gnd, a);
    pmos #(tdelay) p5(a_, vdd, a);

    nmos #(tdelay) n6(b_, gnd, b);
    pmos #(tdelay) p6(b_, vdd, b);


    nmos #(tdelay) n1(out, i1, a);
    nmos #(tdelay) n2(i1, gnd, b);
    nmos #(tdelay) n3(out, i2, a_);
    nmos #(tdelay) n4(i2, gnd, b_);

    pmos #(tdelay) p1(out, i3, a);
    pmos #(tdelay) p2(out, i3, b);
    pmos #(tdelay) p3(i3, vdd, a_);
    pmos #(tdelay) p4(i3, vdd, b_);

endmodule

module testXor();
    wire out;
    reg a, b;
    reg pass;

    xortest x1(out, a, b);

    initial begin
	pass = 1'b1;
	a=1;b=1;
	#100; $display("A=%b  B=%b Out=%b",a,b,out);
	a=0;b=1;
	#100; $display("A=%b  B=%b Out=%b",a,b,out);
	a=1;b=0;
	#100; $display("A=%b  B=%b Out=%b",a,b,out);
	a=0;b=0;
	#100; $display("A=%b  B=%b Out=%b",a,b,out);

	repeat (3) begin
	    a=0;b=1;
	    #100; $display("REP A=%b  B=%b Out=%b",a,b,out);
	    a=1;b=0;
	    #100; $display("REP A=%b  B=%b Out=%b",a,b,out);
	end

	a=1;b=1;
	#100; $display("A=%b  B=%b Out=%b",a,b,out);
	a=0;b=1;
	#100; $display("A=%b  B=%b Out=%b",a,b,out);
	a=1;b=0;
	#100; $display("A=%b  B=%b Out=%b",a,b,out);
	a=0;b=0;
	#100; $display("A=%b  B=%b Out=%b",a,b,out);

	if (pass) $display("PASSED");
    end

    always @(out) begin
	// Wait for the value to settle.
	#10 if (out !== (a ^ b)) begin
	    $display("Failed at %0t, expected %b, got %b, with a=%b, b=%b",
	             $time, a ^ b, out, a, b);
	    pass = 1'b0;
	end
    end
endmodule
