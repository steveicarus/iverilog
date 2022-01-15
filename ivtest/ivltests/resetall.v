module top_default;
  initial begin
    $printtimescale(top_default);
    $printtimescale(top_timescale);
    $printtimescale(top_resetall);
    $printtimescale(top_timescale2);
    $printtimescale(top_timescale3);
  end
endmodule

`resetall
`timescale 1ns/1ns
module top_timescale;
  reg a;
  initial a = 1'b1;
endmodule

`resetall
`resetall
module top_resetall;
  reg a;
  initial a = 1'b0;
endmodule

`resetall
`timescale 1ms/1ms
module top_timescale2;
  reg a;
  initial a = 1'b1;
endmodule

`resetall
`timescale 1us/1us
module top_timescale3;
  reg a;
  initial a = 1'bz;
endmodule
