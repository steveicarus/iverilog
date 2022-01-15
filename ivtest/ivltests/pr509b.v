module whoever_wrote_this_should_be_shot ( Q, D, G );
    output   Q;
    input    D, G;

    wire     Q_int;

    assign ( pull0, pull1 ) Q_int = Q_int;

    bufif1   buf_D ( Q_int, D, G );
    buf      buf_Q ( Q, Q_int );
endmodule

module testbench;
    wire     Q;
    reg      D, G;

    whoever_wrote_this_should_be_shot uut ( Q, D, G );

    initial begin
        D = 1'b0;
        forever #5 D = ~ D;
    end

    initial begin
        G = 1'b0;
        forever #27 G = ~ G;
    end

    initial begin
        $monitor( $time,,,G,,,D,,,Q );

        // time 28: G=1, D=1, Q=1
        #28 if (Q !== 1) begin
	   $display("FAILED -- Q should be 1, is %b", Q);
	   $finish;
	end

        // time 31: G=1, D=0, Q=0
        #3 if (Q !== 0) begin
	   $display("FAILED -- Q should be 0, is %b", Q);
	   $finish;
	end

        // time 51: G=1, D=0, Q=0
        #20 if (Q !== 0) begin
	   $display("FAILED -- Q should be 0, is %b", Q);
	   $finish;
	end

        // time 56:  G=0, D=1, Q=0
        #5 if (Q !== 0) begin
	   $display("FAILED -- Q should be 0, is %b", Q);
	   $finish;
	end

        // time 82:  G=1, D=0, Q=0
        #26 if (Q !== 0) begin
	   $display("FAILED -- Q should be 0, is %b", Q);
	   $finish;
	end

        // time 86:  G=1, D=1, Q=1
        #5 if (Q !== 1) begin
	   $display("FAILED -- Q should be 1, is %b", Q);
	   $finish;
	end

        #1000 $display("PASSED");
        $finish;
    end
endmodule
