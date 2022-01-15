// Regression test for bug reported by Niels Moeller on
// 15-Mar-2015 via the iverilog-devel mailing list.
module test();

wire [7:0] my_net;

assign my_net[3:0] = 1;
assign my_net[7:4] = 2;

initial begin
  #1 $monitor("At time %0t, field 1 = %h, field 2 = %h",
              $time, my_net[3:0], my_net[7:4]);
  #1 $finish(0);
end

endmodule
