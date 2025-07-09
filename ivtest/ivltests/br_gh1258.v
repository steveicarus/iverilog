module DFF (output reg Q, input CLK, D);
        parameter [0:0] INIT = 1'b0;
        initial Q = INIT;

        specify
                (posedge CLK => (Q : D)) = (480, 660);
                $setup(D, posedge CLK, 576);
        endspecify

        always @(posedge CLK)
                Q <= D;
endmodule
