module top;
  reg [7:0] array [1:0];
  reg \array[0] ;
  reg \array[1] ;
  integer idx;

  initial begin
    $dumpfile("work/dup.vcd");
    $dumpvars(0, array[0]);
    idx = 1;
    $dumpvars(0, array[idx]);
    $dumpvars(0, top);
    array[0] = 8'd0;
    #1
    array[0] = 8'd1;
    #1
    \array[0] = 1'b1;
    \array[1] = 1'b0;
  end
endmodule
