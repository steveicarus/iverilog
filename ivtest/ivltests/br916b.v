module top;
  integer value;

  initial $monitor("%0d %0d", value, value + 1);

  initial begin
    #1 value = 0;
    #1 value = 2;
  end
endmodule
