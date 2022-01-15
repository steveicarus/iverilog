module test();

typedef bit [3:0] array_t[];

array_t array;

initial begin
  array = 8'd1 << 4;
end

endmodule
