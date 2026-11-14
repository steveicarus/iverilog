module top;
  string str = "Hello";
  string idx = "invalid";
  byte chr1 = str[idx];

  initial begin
    str[idx] = "a";
    $display("FAILED: should have a compile error.");
  end
endmodule
