`timescale 1us/1us
module top_timescale;
  initial begin
    $printtimescale(top_timescale);
    $printtimescale(top_timescale2);
  end
endmodule

`resetall
`timescale 1ns/1ns
module top_timescale2;
  reg a;
  initial a = 1'b1;
endmodule
