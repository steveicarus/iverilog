module test();

wire [7:0] value1;
reg  [7:0] value2;

reg en;

assign value1[3:0] = 4'b1010;

always @* begin
  if (en)
    value2 <= value1;
end

(* ivl_synthesis_off *)
initial begin
  #1 en = 0;
  #1 en = 1;
  #1 en = 0;
  $display("%b %b", value1, value2);
  if (value2 === 8'bzzzz1010)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
