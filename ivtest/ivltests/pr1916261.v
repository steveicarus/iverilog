module top;
  reg pass = 1'b1;

  parameter one  = 1'b1;
  parameter zero = 1'b0;

  wire [3:0] ca_tru = one  ? 4'b0001 : 4'b0000;
  wire [3:0] ca_fal = zero ? 4'b0000 : 4'b0010;

  initial begin
    #1;
    if (ca_tru != 4'b0001) begin
      $display("FAILED: CA true expression (%b != 4'b0001)", ca_tru);
      pass = 1'b0;
    end

    if (ca_fal != 4'b0010) begin
      $display("FAILED: CA false expression (%b != 4'b0010)", ca_fal);
      pass = 1'b0;
    end

    if(pass) $display("PASSED");
  end
endmodule
