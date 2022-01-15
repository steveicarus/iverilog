module top;
  reg [7:0] arr [4:-2];

  initial begin
    $dumpfile("work/test.vcd");
    $dumpvars(1, arr[4]);
    arr[4] = 8'h00;
    #1 arr[4] = 8'hff;
  end
endmodule
