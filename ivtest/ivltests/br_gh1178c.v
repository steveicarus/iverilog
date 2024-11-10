module net_connect #(parameter WIDTH=1) (wo, w);
  output wire[WIDTH-1:0] wo;
  input  wire[WIDTH-1:0] w;
  assign wo = w;
endmodule


module ReplicateMod (
  input  wire [3:0] bus,
  output wire [7:0] replicated
  );
  net_connect #(.WIDTH(8)) net_connect (replicated, ({2{bus}}));
endmodule


module tb;
  logic passed;
  logic [3:0] bus;
  wire  [7:0] replicated;
  wire  [3:0] wire__bus = bus;

  ReplicateMod dut(.bus(wire__bus), .replicated(replicated));

  initial begin
    passed = 1'b1;
    bus = 4'h3;
    #1
    if(replicated !== 8'h33) begin
      passed = 1'b0;
      $display("FAILED: Expected 'h33, but found 'h%x with inputs 'h%x", replicated, bus);
    end
    #1
    bus = 4'hc;
    #1
    if(replicated !== 8'hcc) begin
      passed = 1'b0;
      $display("FAILED: Expected 'hcc, but found 'h%x with inputs 'h%x", replicated, bus);
    end
    #1
    if (passed) $display("PASSED");
  end
endmodule
