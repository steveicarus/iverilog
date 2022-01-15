module main;
  reg pass;

  genvar i;
  generate
    for( i=1; i<3; i=i+1 )
      begin : U
        reg [1:0] x;
      end
    for( i=0; i<2; i=i+1 )
      begin : V
        initial begin
          U[(i+1)%4].x = 2'd0;
          #5;
          U[(i+1)%4].x = i;
        end
      end
  endgenerate

  initial begin
    pass = 1'b1;
    #4;
    if (U[1].x != 2'd0) begin
      $display("Failed to clear U[1].x, got %b", U[1].x);
      pass = 1'b0;
    end
    if (U[2].x != 2'd0) begin
      $display("Failed to clear U[2].x, got %b", U[1].x);
      pass = 1'b0;
    end
    #2;
    if (U[1].x != 2'd0) begin
      $display("Failed to set U[1].x, expected 2'd0, got %b", U[1].x);
      pass = 1'b0;
    end
    if (U[2].x != 2'd1) begin
      $display("Failed to set U[2].x, expected 2'd1, got %b", U[1].x);
      pass = 1'b0;
    end
    if (pass) $display("PASSED");
  end
endmodule
