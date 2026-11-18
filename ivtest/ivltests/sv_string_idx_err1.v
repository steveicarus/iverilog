module top;
  string str = "Hello";
  byte chr1 = str[];
  byte chr2 = str[1, 2];

  initial begin
    str[] = "a";
    str[1, 2] = "b";
    $display("FAILED: should have a compile error.");
  end
endmodule
