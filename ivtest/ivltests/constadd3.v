module test;
    integer i;
    initial begin
        i = 7+1;
        if (i != 8)
            $display ("FAILED -- i = %0d != 8", i);
        else
            $display ("PASSED");
    end
endmodule
