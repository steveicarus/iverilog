`begin_keywords "1364-2005"
module top;
  reg [31:0] var;
  integer ivar;

  wire [31:0] out = ivar + 2147483648;

  initial begin
    $monitor(var,, out);
    var = 0;
    ivar = -2147483648;
    repeat (60) begin
       #1 var = $urandom;
       ivar = $random;
    end
  end
endmodule
`end_keywords
