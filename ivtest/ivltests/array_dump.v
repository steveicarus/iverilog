module top;
  integer lp;
  reg [7:0] array [2:0];

  initial begin
    // We can use the following to dump arrays. We can not dump
    // a whole array with this statment "$dumpvars(0, array);".
    //
    // $dumpvars is special in that it converts a variable array
    // select &A<array, index> into a constant array select.
    // This is needed to make the following work as expected.
    $dumpfile("work/array_dump.vcd");
    for (lp = 0; lp < 3; lp = lp+1) $dumpvars(0, array[lp]);
    #1;
    array[0] = 8'hff;
    array[1] = 8'h00;
    array[2] = 8'h55;
  end
endmodule
