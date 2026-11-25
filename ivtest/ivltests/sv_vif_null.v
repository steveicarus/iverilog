// Check that a read through a null virtual interface stops the simulation
// with a fatal error that gives the file and line of the access.

interface bus_if;
  logic [7:0] data = 8'h5a;
endinterface

module test;
  class holder;
    virtual bus_if vif;
  endclass

  bus_if a();
  holder h;
  logic [7:0] val;

  initial begin
    h = new;
    h.vif = a;
    val = h.vif.data;
    h.vif = null;
    #1 val = h.vif.data;
    $display("FAILED: read through null continued, val=%h", val);
  end
endmodule
