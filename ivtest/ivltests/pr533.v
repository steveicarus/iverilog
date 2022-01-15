module example;
    reg          r, c, e;
    reg   [4:0]  a, b;
    wire         d;

    assign d = ( r | ( a == b ) ) ? 1'b0 : 1'b1;

    // Change inputs at time n*100

    initial begin
        #100 r = 1'bx; a = 5'bxxxxx; b = 5'bxxxxx;
        #100 r = 1'b1; a = 5'bxxxxx; b = 5'bxxxxx;
        #100 r = 1'b1; a = 5'b00000; b = 5'b00000;
        #100 r = 1'b0; a = 5'b00000; b = 5'b00000;
        #100 $finish(0);
    end

    // Store c and e at time n*100 + 25.
    // Note that the value assigned to c is exactly the same as
    // the continuous assignment RHS for d (assigned to e).

    initial #25 forever begin
        #100
        c = ( r | ( a == b ) ) ? 1'b0 : 1'b1;
        e = d;
    end

    // Display all values at time n*100 + 50

    initial #50 forever begin
        #100
        $display( "%b,%b,%b = ( %b | ( %b == %b ) ) ? 0 : 1",
                  c, d, e, r, a, b );
    end

endmodule
