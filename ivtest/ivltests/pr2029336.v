/* testcase  */

/**
generate this input file:
$ cat t.in
1073741824
2147483648
4294967296
$

expected output in t.out:
$ cat t.out
# x = 1073741824
# x = 2147483648
# x = 4294967296
         0
1073741824
2147483648
4294967296

Icarus Verilog output in t.out:
$ cat t.out
# x = 1073741824
# x = 2147483648
# x = 4294967296
         0
1073741824
6442450944
         0
*/

module testbench;
	parameter WIDTH = 33;
	reg clk;
	reg [WIDTH-1:0] in;
	reg [WIDTH-1:0] test_val1;
	reg [WIDTH-1:0] test_val2;
	reg [WIDTH-1:0] test_val3;
	integer infile, outfile, count;

	initial begin
		clk = 0;
		in = 0;
		test_val1 = 1 << 30;
		test_val2 = 1 << 31;
		test_val3 = 1 << 32;
		infile = $fopen("ivltests/pr2029336.in", "r");
		outfile = $fopen("work/pr2029336.out", "w");
		$fwrite(outfile, "# x = %d\n", test_val1);   // $fwrite() seems to be ok...
		$fwrite(outfile, "# x = %d\n", test_val2);   // $fwrite() seems to be ok...
		$fwrite(outfile, "# x = %d\n", test_val3);   // $fwrite() seems to be ok...
	end

	always @(negedge clk) begin
		$fwrite(outfile, "%d\n", in);
		count = $fscanf(infile, "%d\n", in);  // $fscanf() seems buggy...
		if (count != 1) begin
			$fclose(infile);
			$fclose(outfile);
			$finish;
		end
	end

	always #1
		clk = ~clk;

endmodule
