// Check that continuous array assignment for multi-dimensional arrays behaves
// correctly when left-to-right order is reversed.

module test;

  logic [31:0] x[2:0][6:0][4:0];
  wire [31:0] y[0:2][6:0][0:4];

  assign y = x;

  initial begin
    static integer idx = 1;
    static bit failed = 1'b0;

    foreach (x[i,j,k]) begin
      x[i][j][k] = idx;
      idx++;
    end

    #1

    idx = 1;
    foreach (y[i,j,k]) begin
      $display(i, j, k, y[i][j][k]);
      if (y[i][j][k] !== idx) begin
        $display("FAILED");
        failed = 1'b1;
      end
      idx++;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
