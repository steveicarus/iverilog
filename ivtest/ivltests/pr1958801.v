// pr1958001

module s_cmpGe( in00, in01, out00 );

    parameter bw_in00  = 32;
    parameter bw_in01  = 32;

    input signed [bw_in00-1:0]  in00;
    input signed [bw_in01-1:0]  in01;
    output out00;

    assign out00 = ( in00 >= in01 );
endmodule

module x;

    reg signed [31:0] a;
    reg signed b;
    wire c;

    s_cmpGe #(32, 1) inst(a, b, c);

    initial
    begin
        b = 0;
        a = -1;

        #1;

        $display("%d >= %d = %d", a, b, c);
        if (c !== 0) begin
	   $display("FAILED");
	   $finish;
	end

        $display("PASSED");
    end

endmodule
