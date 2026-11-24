// Check that calling a function through a virtual interface gives a sorry.
interface bus_if #(parameter W = 8);
  logic [W-1:0] data;
  event ev;
  localparam LP = 3;
  function int f(); return 7; endfunction
endinterface
class cfg_c;
  virtual bus_if vif;
endclass

module test;
  bus_if b0();
  virtual bus_if v;
  cfg_c c;
  int x;
  initial begin
    v = b0;
    c = new;
    c.vif = b0;
    x = v.f();
    x = c.vif.f();
    $display("FAILED");
  end
endmodule
