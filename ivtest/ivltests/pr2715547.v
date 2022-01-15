module top;
  reg [31:0] mem [3:0];

  initial begin
    mem[1] = {32{1'b1}};
    mem[1][15] = 1'b0;

    if (mem[1] !== 32'hffff7fff) $display("Failed, got %h", mem[1]);
    else $display("PASSED");
  end
endmodule
