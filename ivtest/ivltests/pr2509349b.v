module top;
  reg [79:0] str;

  initial begin
    $readmempath(str);
    str = "test";
    str[7:0] = 'd2;
    $readmempath(str);
  end
endmodule
