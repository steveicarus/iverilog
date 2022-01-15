// This is the second part of a two-part test that checks that the argument to
// a $signed or $unsigned function is treated as a self-determined expression.
// This part performs tests where the argument is signed.

module pr2922063b;

reg signed [3:0] op1;
reg signed [2:0] op2;
reg        [7:0] result;
reg              fail;

task check_result;

input [7:0] value;

begin
  $write("Expected %b, got %b", value, result);
  if (result !== value) begin
    $write(" *");
    fail = 1;
  end
  $write("\n");
end

endtask

initial begin
  fail = 0;

  $display("-- Addition tests --");

  op1 = 4'b1111; op2 = 3'b111;
  result = 8'sd0 + $signed(op1 + op2);
  check_result(8'b11111110);
  result = 8'sd0 + $unsigned(op1 + op2);
  check_result(8'b00001110);

  op1 = 4'b1000; op2 = 3'b011;
  result = 8'sd0 + $signed(op1 + op2);
  check_result(8'b11111011);
  result = 8'sd0 + $unsigned(op1 + op2);
  check_result(8'b00001011);

  $display("-- Multiply tests --");

  op1 = 4'b0101; op2 = 3'b100;
  result = 8'sd0 + $signed(op1 * op2);
  check_result(8'b11111100);
  result = 8'sd0 + $unsigned(op1 * op2);
  check_result(8'b00001100);

  op1 = 4'b0010; op2 = 3'b100;
  result = 8'sd0 + $signed(op1 * op2);
  check_result(8'b11111000);
  result = 8'sd0 + $unsigned(op1 * op2);
  check_result(8'b00001000);

  $display("-- Left ASR tests --");

  op1 = 4'b1010;
  result = 8'sd0 + $signed(op1 <<< 1);
  check_result(8'b00000100);
  result = 8'sd0 + $unsigned(op1 <<< 1);
  check_result(8'b00000100);

  op1 = 4'b0101;
  result = 8'sd0 + $signed(op1 <<< 1);
  check_result(8'b11111010);
  result = 8'sd0 + $unsigned(op1 <<< 1);
  check_result(8'b00001010);

  $display("-- Right ASR tests --");

  op1 = 4'b0101;
  result = 8'sd0 + $signed(op1 >>> 1);
  check_result(8'b00000010);
  result = 8'sd0 + $unsigned(op1 >>> 1);
  check_result(8'b00000010);

  op1 = 4'b1010;
  result = 8'sd0 + $signed(op1 >>> 1);
  check_result(8'b11111101);
  result = 8'sd0 + $unsigned(op1 >>> 1);
  check_result(8'b00001101);

  if (fail)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
