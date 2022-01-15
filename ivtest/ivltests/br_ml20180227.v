module test();

string str;
reg [127:0] bitstr;

initial begin
  str = "hello";
  bitstr = str;
end

endmodule
