/*
 * Based on bug report PR#842.
 */

module test;
    foo _foo();

    initial
        begin
            // Access the FOO parameter inside the _foo instance ?
            $display("%d", _foo.FOO);
	    if (_foo.FOO != 17) begin
	       $display("FAILED -- _foo.FOO=%d", _foo.FOO);
	       $finish;
	    end
	    $display("PASSED");
        end
endmodule

module foo;
    parameter FOO = 17;
endmodule
