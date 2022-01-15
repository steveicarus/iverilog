module bug();

  reg [95:0] e;
  reg [95:0] f;
  initial e = 96'd10;
  initial f = 96'hAAAAAAAAAAAAAAAAAAAAAAAA;

  wire [95:0] div = e / f ;
  wire [95:0] mod = e % f ;  // also fails

  initial begin
     #1 $display("div=%h", div);
        $display("mod=%h", mod);

     if (div !== 96'd0) begin
	$display("FAILED");
	$finish;
     end

     if (mod !== 96'd10) begin
	$display("FAILED");
	$finish;
     end

     $display("PASSED");
     $finish;
  end

endmodule
