module test;
    event e;
    initial begin
	repeat (5) begin
	    #10;
	    ->e;
	end
    end
endmodule
