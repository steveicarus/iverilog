module test();

localparam value = {16384{4'b1001}};

wire [65535:0] q = value;

initial begin
  if (q === value)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
