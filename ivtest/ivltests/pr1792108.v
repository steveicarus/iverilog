`begin_keywords "1364-2005"
module main;
  wire y1, y2, y3;
  reg a;

  initial begin
    $monitor($time , " y1 = %d, y2 = %d, y3 = %d, a = %d", y1, y2, y3, a);
    #1 a = 1;
    #1 a = 0;
  end

  sub s1(y1, y2, y3, a);

endmodule // main

module sub(y1, y2, y3, a);
  output y1, y2, y3;
  input a;
  reg y1, y2, y3;
  reg int;

  always @(*) begin
    y1 <= a;
    y2 <= y1;

    int <= a;
    y3 <= int;
  end

endmodule
`end_keywords
