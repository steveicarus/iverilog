// Copyright 2007, Martin Whitaker.
// This code may be freely copied for any purpose.

module duplicate_names();

localparam up = 1;

generate
  if (up)
    begin:block1
      wire [2:0] count1;
      count_up   counter(count1);
    end
endgenerate

initial begin:block1
  reg [2:0] count2;

  #1 count2 = 4;
  #1 count2 = 5;
  #1 count2 = 6;
  #1 count2 = 7;
end

endmodule

module count_up(output reg [2:0] count);

initial begin
  #1 count = 0;
  #1 count = 1;
  #1 count = 2;
  #1 count = 3;
end

endmodule
