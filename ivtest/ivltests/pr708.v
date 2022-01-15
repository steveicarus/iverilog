module test;

    parameter PARM = 1.5;
    reg r;

    initial begin
        case (PARM)
	    1.0 : r <= 'd1;
            1.5 : r <= 'd0;
            2.0 : r <= 'd1;
            default: r <= 1'bx;
        endcase

        #1;

        if (r !== 'd0)
            $display("FAILED %b != 0", r);
        else
            $display("PASSED");
    end

endmodule
