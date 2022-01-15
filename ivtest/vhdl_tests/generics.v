// A few simple tests of translating parameters to generics
module top();
  wire [7:0] v1, v2, v3;
  wire [7:0] w1, w2, w3;

  child c1(v1, w1);
  child c2(v2, w2);
  child c3(v3, w3);

  defparam c1.MY_VALUE = 6;
  defparam c2.MY_VALUE = 44;

  initial begin
    #2;
    $display("c1 reg value: %d", v1);
    $display("c2 reg value: %d", v2);
    $display("c3 reg value: %d", v3);
    $display("c1 wire value: %d", w1);
    $display("c2 wire value: %d", w2);
    $display("c3 wire value: %d", w3);
    if (v1 !== 6)
      $display("FAILED - v1 !== 6");
    else if (v2 !== 44)
      $display("FAILED - v2 !== 44");
    else if (v3 !== 12)
      $display("FAILED - v3 !== 12");
    else if (w1 !== 7)
      $display("FAILED - v1 !== 7");
    else if (w2 !== 45)
      $display("FAILED - v2 !== 45");
    else if (w3 !== 13)
      $display("FAILED - v3 !== 13");
    else
      $display("PASSED");
  end

endmodule // top

module child(value, value_w);
  output [7:0] value, value_w;
  reg [7:0]    value;

  parameter MY_VALUE = 12;

  assign value_w = MY_VALUE + 1;

  // Make a non-trivial process
  initial begin
    #1;
    value <= MY_VALUE;
  end
endmodule // child
