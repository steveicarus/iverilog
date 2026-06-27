// Check that a genvar name can shadow a visible type identifier.

typedef int G;

module test;
  reg failed;

  genvar G;
  generate
    for (G = 0; G < 2; G = G + 1) begin : gen_block
      localparam int VALUE = G;
    end
  endgenerate

  initial begin
    failed = 1'b0;

    if (gen_block[0].VALUE != 0 || gen_block[1].VALUE != 1) begin
      $display("FAILED(%0d). genvar name did not hide typedef", `__LINE__);
      failed = 1'b1;
    end

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
