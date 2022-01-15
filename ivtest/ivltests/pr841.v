/*
 * Based on bug report PR#841
 */

module err ();

reg clk;
initial begin
      clk = 1'b1;
      #3 forever #10 clk=~clk;
end

always @(posedge clk) begin $display("PASSED"); $finish(0); end

//wire kuku=clk;
pll pll (clk);

endmodule


module pll (inclk0);
input  inclk0;

tri0 inclk0;

endmodule

//this is example of running bug and not compilation bug.
//if you will try to run it (a.out) after the compilation (iverilog try_err.v)
//you will get an error ("./a.out:7: parse error")
//from some unknown reason line 11 solve this problem.
