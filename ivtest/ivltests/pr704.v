/* PR#704 */
module foo;

  reg [80*8:1]     filename;
  reg [31:0]       memory[1:2048];

  initial filename = "ivltests/pr704.hex";

  initial begin
     $display("The filename is %0s", filename);
     $readmemb(filename, memory, 1);

     if (memory[1] !== 32'haa_aa_aa_aa) begin
	$display("FAILED");
	$finish;
     end

     if (memory[2] !== 32'h55_55_55_55) begin
	$display("FAILED");
	$finish;
     end

     $display("PASSED");
     $finish;
  end

endmodule
