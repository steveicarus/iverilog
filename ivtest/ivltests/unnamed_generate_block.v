// Copyright 2007, Martin Whitaker.
// This code may be freely copied for any purpose.

module unnamed_generate_block();

localparam up = 1;

wire [2:0] count1;
wire [2:0] count2;
wire [2:0] count3;

generate
  if (up)
    count_up   counter(count1);
  else
    count_down counter(count1);
endgenerate

generate
  if (up)
    begin:genblk1
      count_up   counter(count2);
    end
  else
    begin:genblk1
      count_down counter(count2);
    end
endgenerate

count_down genblk01(count3);

initial begin:genblk001
  reg [2:0] count;

  #1 count = 4;
  #1 count = 5;
  #1 count = 6;
  #1 count = 7;
end

always @(genblk0001.counter.count) begin
  $display(genblk0001.counter.count);
end

//initial begin
//  $dumpfile("dump.vcd");
//  $dumpvars;
//end

endmodule

module count_up(output reg [2:0] count);

initial begin
  #1 count = 0;
  #1 count = 1;
  #1 count = 2;
  #1 count = 3;
end

endmodule

module count_down(output reg [2:0] count);

initial begin
  #1 count = 3;
  #1 count = 2;
  #1 count = 1;
  #1 count = 0;
end

endmodule
