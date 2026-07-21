// Virtual-interface vertical slice (UVM Tier A #3).
// Note: this slice requires the `interface` keyword in the type
// (`virtual interface bus_if`) to avoid parser conflicts with `virtual class`.
interface bus_if;
  logic clk;
  logic [7:0] data;
endinterface

class driver;
  virtual interface bus_if vif;
  function new(virtual interface bus_if i);
    vif = i;
  endfunction
  task drive(logic [7:0] d);
    @(posedge vif.clk);
    vif.data = d;
  endtask
endclass

module top;
  bus_if bif();
  driver drv;
  initial begin
    bif.clk = 0;
    forever #5 bif.clk = ~bif.clk;
  end
  initial begin
    drv = new(bif);
    #12 drv.drive(8'hA5);
    if (bif.data !== 8'hA5) $fatal(1, "FAILED");
    $display("PASSED");
    $finish;
  end
endmodule
