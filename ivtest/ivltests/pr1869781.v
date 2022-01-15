module dummy;

integer         i;
integer         foo_value;
reg     [1:0]   foo_bit;

initial
begin
        i = 1;
        foo_value = 10;
        foo_bit[i] <= #foo_value  1'b0;

        /*
                NOTE:
                if you replace previous line either with:
                        foo_bit[1] <= #foo_value  1'b0;
                or with:
                        foo_bit[i] <= #10  1'b0;
                then "invalid opcode" is not shown
        */

        #20 if (foo_bit[1] !== 1'b0) begin
	   $display("FAILED -- foo_bit[1] = %b", foo_bit[1]);
	   $finish;
	end
        $display("PASSED");
end

endmodule
