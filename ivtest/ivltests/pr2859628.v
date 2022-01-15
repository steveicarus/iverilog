module top;
  reg [3:0] array [0:1];

  initial begin
    $dumpfile("work/pr2859628.vcd");
    $dumpvars(0, top);
    // This will complain that the array words have already been included!
    // They have not been since array words are only explicitly added.
    // The word/scope check code needs to be updated to ignore array words.
    $dumpvars(0, array[0], array[1]);
    #1;
  end
endmodule
