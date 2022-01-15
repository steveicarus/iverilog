module lvl3;
    reg [1:0] m[1:0];
    initial begin
	fork: my_fork
	    repeat (1) begin
		m[0] = 2'b0;
	    end
	    repeat (1) begin
		m[1] = 2'b1;
	    end
	join
    end
endmodule


module lvl2_0;
    reg r;
    initial r = $random;
    lvl3 lvl3();
endmodule

module lvl1_0;
    reg r;
    function f_foo;
	input bar;
    begin
	f_foo = bar;
    end
    endfunction
    initial r = f_foo(r);
    lvl2_0 lvl2();
endmodule

module top0;
    reg r;
    task t_bar;
	r = 1'b0;
    endtask
    initial begin: my_init
	r = $random;
	t_bar;
    end
    lvl1_0 lvl1();
endmodule


module lvl2_1;
    integer i;
    initial i = $random;
    lvl3 lvl3();
endmodule

module lvl1_1;
    integer i;
    initial i = $random;
    lvl2_1 lvl2();
endmodule

module top1;
    integer i;
    initial i = $random;
    lvl1_1 lvl1();
endmodule

module top2;
    initial $test;
endmodule
