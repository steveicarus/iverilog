// Adapted from test case submitted by Geoff Blackman

module pr2276163();
    function automatic  integer f1;
        input integer in;
        f1 = in + 1;
    endfunction

    function  integer f2;
        input integer in;
        f2 = in * 2;
    endfunction

    integer ret;
    initial begin
        ret = f1 ( f1 (1) );
        if (ret !== 3) begin
            $display("FAILED: expected 3, got %0d", ret);
            $finish;
        end
        ret = f1 ( f2 (2) );
        if (ret !== 5) begin
            $display("FAILED: expected 5, got %0d", ret);
            $finish;
        end
        ret = f2 ( f1 (3) );
        if (ret !== 8) begin
            $display("FAILED: expected 8, got %0d", ret);
            $finish;
        end
        ret = f2 ( f2 (4) );
        if (ret !== 16) begin
            $display("FAILED: expected 16, got %0d", ret);
            $finish;
        end
        $display("PASSED");
    end
endmodule
