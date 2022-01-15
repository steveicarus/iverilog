module port_test(a);
  parameter p_w=1<<5;  // 32
  parameter c_w=p_w>>4;//  2  (<--- here)
  output [c_w-1:0] a;
  wire   [c_w-1:0] a='h0;

  initial begin
     $display("p_w=%b, c_w=%b", p_w, c_w);

     if (c_w !== 2) begin
	$display("FAILED -- c_w == %b", c_w);
	$finish;
     end

     if ($bits(a) !== 2) begin
	$display("FAILED -- $bits(a) == %b", $bits(a));
	$finish;
     end

     $display("PASSED");
  end // initial begin

endmodule

module main;
  wire [1:0] a;
  port_test m (.a(a));
endmodule
