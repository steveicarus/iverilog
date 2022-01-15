module top;
  reg pass;
  reg [3:0] array [1:8];
  reg signed [2:0] a;
  reg signed [127:0] b;
  reg [4*8:1] res;

  initial begin
    array [7] = 4'b1001;
    pass = 1'b1;
    /* If this fails it is likely because the index width is less
     * than an integer width. */
    a = -1;
    $sformat(res, "%b", array[a]);
    if (res !== "xxxx") begin
      $display("Failed: &A<> negative, expected 4'bxxxx, got %s.", res);
      pass = 1'b0;
    end

    b = 7;
    b[120] = 1'b1; // This should be stripped!
    $sformat(res, "%b", array[b]);
    if (res !== "1001") begin
      $display("Failed: &A<> large, expected 4'b1001, got %s.", res);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
