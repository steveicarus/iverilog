module net_connect #(parameter WIDTH=1) (w, w);
  inout wire[WIDTH-1:0] w;
endmodule


module ReplicateMod (
  inout wire [3:0] bus,
  inout wire [7:0] replicated
  );
  // It is illegal to connect a replication to an output or inout port.
  net_connect #(.WIDTH(8)) net_connect (replicated, ({2{bus}}));
endmodule


module tb;
  logic [3:0] bus;
  wire [7:0] replicated;
  wire [3:0] wire__bus = bus;
  ReplicateMod dut(.bus(wire__bus), .replicated(replicated));
  initial $display("FAILED: this should be a compilation error.");
endmodule
