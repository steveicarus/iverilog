module top;
  wire [3:0] array [1:0];
  integer sel;

  assign array[0] = 4'h0;
  assign array[1] = 4'h1;

  initial begin
    #1;
    $display(" %h %h", array[0], array[1]);
    // This is only a problem for a wire (net array)!
    sel = 0;
    $display(" %h %h", array[sel], array[sel+1]);
    $display("PASSED");
  end
endmodule
