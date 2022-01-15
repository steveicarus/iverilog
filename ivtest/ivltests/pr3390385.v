module tb;

reg [1:0] i, j;
reg [3:0] x[0:2];
reg error;

initial begin
   error = 0;

   i = 0;
   j = i++;
   if (i !== 2'b01 || j !== 2'b00) begin
      $display("FAILED j = i++ --> j=%b, i=%b", j, i);
      error = 1;
   end

   i = 0;
   x[0] = 4'dx;
   x[1] = 4'dx;
   x[i++] = 0;
   if (x[0] !== 4'd0 || x[1] !== 4'dx) begin
      $display("FAILED x[i++] = 0 --> x[0]=%b, x[1]=%b, i=%b", x[0], x[1], i);
      error = 1;
   end

   i = 0;
   x[0] = 1;
   x[i++] += 2;
   if (x[0] !== 4'd3) begin
      $display("FAILED x[0] should be 3, but it is %d.", x[0]);
      error = 1;
   end
   if (i !== 2'd1) begin
      $display("FAILED i should be 1, but it is %d.", i);
      error = 1;
   end
   if (error == 0)
     $display("PASSED");
end
endmodule // tb
