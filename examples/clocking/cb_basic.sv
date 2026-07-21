// Clocking-block vertical slice (UVM Tier A #4).
// Interface-local clocking with @(bif.cb), NBA write, and sampled read.
interface bus_if(input logic clk);
  logic [7:0] data;
  logic req;
  clocking cb @(posedge clk);
    input  #0 data;
    output #0 req;
  endclocking
endinterface

module top;
  logic clk;
  bus_if bif(clk);
  logic [7:0] x;
  initial begin clk = 0; forever #5 clk = ~clk; end
  initial begin
    bif.data = 8'h3C;
    @(bif.cb);
    bif.cb.req <= 1'b1;   // NBA to underlying req (#0 skew)
    x = bif.cb.data;       // #0 sample of data
    @(bif.cb);             // next clocking event: NBA is visible on bif.req
    if (x !== 8'h3C || bif.req !== 1'b1) begin
      $display("FAILED x=%0h req=%0b", x, bif.req);
      $finish;
    end
    $display("PASSED");
    $finish;
  end
endmodule
