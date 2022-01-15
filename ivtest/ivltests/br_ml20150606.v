module dut(input [3:0] DataI, output [3:0] DataO);

wire [3:0] DataI;
reg  [3:0] DataO;

always @* DataO = DataI;

endmodule

module top();

reg  [3:0] DataI;
wire [3:0] DataO;

dut dut(DataI, DataO);

initial begin
  DataI = 5;
  #1;
  if (DataO === 5)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
