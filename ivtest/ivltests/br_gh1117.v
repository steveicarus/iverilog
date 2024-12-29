module top;
  wire x, y, z;
  reg in;
  genvar i;

  generate
  for (i=0; i<1; i=i+1) begin
    assign x = in;
  end
  generate // This should be an error
  for (i=0; i<1; i=i+1) begin
    assign y = in;
  end
  endgenerate
  endgenerate

  generate // This is ok
  for (i=0; i<1; i=i+1) begin
    assign z = in;
  end
  endgenerate

  initial $display("Failed: should be a compile error!");

endmodule
