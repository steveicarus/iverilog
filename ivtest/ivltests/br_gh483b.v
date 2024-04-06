module Module;
    parameter T = 10;
    wire [T-1:0] x = 8'h55;
    initial $display("Module %b %0d", x, T);
endmodule

module top;
    wire [7:0] x;
    Module #($bits(x)) mA();
    Module #(8) mB();

    initial begin
        if (mA.x === 8'h55 && mB.x === 8'h55)
            $display("PASSED");
        else
            $display("FAILED");
    end
endmodule
