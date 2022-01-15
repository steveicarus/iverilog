module dut(input wire [7:0] i = 8'd10, output wire [7:0] o);

assign o = i;

endmodule

module tb();

wire [7:0] result;

dut dut(.o(result));

initial begin
  #1;
  if (result === 10)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
