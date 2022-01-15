module Top;

generate
  genvar i;

  for (i = 0; i < 1; i = i + 1) begin
    Sub1 SubMod1();
  end
endgenerate

endmodule


module Sub1;

wire [7:0] Value;

Sub2 SubMod2(Value);

defparam SubMod2.Width = 8;

initial begin
  #1;
  $display("Value = %h", Value);
  if (Value === 8'hff)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule


module Sub2(Out);

parameter Width = 4;

output [Width-1:0] Out;

assign Out = {Width{1'b1}};

endmodule
