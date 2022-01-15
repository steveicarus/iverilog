module param_width();

parameter       a = 3'd4;
parameter       b = 3'd5;
parameter       c = 3'd4;
parameter       d = 3'd5;

parameter [3:0] sum1 = a + b;
parameter       sum2 = a + b;
parameter [3:0] sum3 = c + d;
parameter       sum4 = c + d;

defparam c = 2'd2;
defparam d = 2'd3;

initial begin
  $display("%b", sum1);
  $display("%b", sum2);
  $display("%b", sum3);
  $display("%b", sum4);
end

endmodule
