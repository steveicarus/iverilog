`define msg(x,y) `"x: `\`"y`\`"`"

module test();

initial begin
  $display(`msg(left side,right side));
end

endmodule
