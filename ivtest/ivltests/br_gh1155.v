module test;

// The test is sensitive to the order in which the code is generated for
// the individual assignments, which currently depends on the alphabetic
// order of the array names. So duplicate the test with the order reversed
// to protect against future compiler changes.

wire [7:0] array1[0:1];
wire [7:0] array2[0:0];

assign array2[0] = 8'h55;

assign array1[0] = { array2[0] };
assign array1[1] = { array2[0] };

wire [7:0] array3[0:0];
wire [7:0] array4[0:1];

assign array3[0] = 8'haa;

assign array4[0] = { array3[0] };
assign array4[1] = { array3[0] };

initial begin
  #0 $display("%h %h", array1[0], array1[1]);
  #0 $display("%h %h", array4[0], array4[1]);
  if (array1[0] === 8'h55 && array1[1] === 8'h55 &&
      array4[0] === 8'haa && array4[1] === 8'haa)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
