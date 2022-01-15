// Test that expression width calculation correctly treats right operand
// of shift as unsigned regardless of its actual type.
module test;
  reg pass;
  reg [8*20:1] str;
  reg signed [3:0] N;

  initial begin
    pass = 1'b1;

    N = -1;
    $sformat(str, "%0d", 1 << N);
    if (str[8*5:1] !== "32768" || str[8*20:8*5+1] !== 0) begin
      $display("FAILED test, expected \"32768\", got \"%0s\"", str);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
