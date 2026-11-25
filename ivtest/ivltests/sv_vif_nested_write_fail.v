// Check that writing a member of a virtual interface through a nested class
// property gives a sorry.
interface bus_if #(parameter W = 8);
  logic [W-1:0] data;
  event ev;
  localparam LP = 3;
  function int f(); return 7; endfunction
endinterface
class cfg_c;
  virtual bus_if vif;
endclass

class drv_c;
  cfg_c c;
  task run();
    c.vif.data = 1;
  endtask
endclass

module test;
  bus_if b0();
  initial $display("FAILED");
endmodule
