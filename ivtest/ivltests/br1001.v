module submod(inout a);

endmodule

module topmod();

wand x;
wor  y;

submod m1(.a(x));
submod m2(.a(y));

initial begin
  if ((x === 1'bz) && (y === 1'bz))
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
