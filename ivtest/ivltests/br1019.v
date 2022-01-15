module sub(input [3:0] value);

wire [3:0] array[1:0];
reg  [3:0] monitor;

assign array[0] = $unsigned(value);

always @(array[0]) begin
  monitor = array[0];
end

endmodule

module top;

wire [3:0] value;

sub sub1(value);
sub sub2(value);

initial begin
  force value = 5;
  #0;
  if ((sub1.monitor === 5) && (sub2.monitor === 5))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
