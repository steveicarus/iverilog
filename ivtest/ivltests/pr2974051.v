// Extended version of original test case, covering part-driven operands
// for all logical operations.
module pr2974051;

wire [7:0] a;
wire [7:0] b;
reg	   c;

assign a[5:2] = 4'b0101;
assign b[5:2] = 4'b1010;

wire [7:0] d = c ? b : a;

wire [7:0] e = a & b;
wire [7:0] f = a | b;
wire [7:0] g = a ^ b;

wire [7:0] h =  a;
wire [7:0] i = ~a;

reg fail;

initial begin
  fail = 0;

  c = 0;
  #1 $display("%b", d);
  if (d !== 8'bzz0101zz) fail = 1;
  c = 1;
  #1 $display("%b", d);
  if (d !== 8'bzz1010zz) fail = 1;

  #1 $display("%b", e);
  if (e !== 8'bxx0000xx) fail = 1;

  #1 $display("%b", f);
  if (f !== 8'bxx1111xx) fail = 1;

  #1 $display("%b", g);
  if (g !== 8'bxx1111xx) fail = 1;

  #1 $display("%b", h);
  if (h !== 8'bzz0101zz) fail = 1;

  #1 $display("%b", i);
  if (i !== 8'bxx1010xx) fail = 1;

  if (fail)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
