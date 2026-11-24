// Check that reading a parameter through a virtual interface gives a sorry.
interface bus_if #(parameter W = 8);
  logic [W-1:0] data;
  event ev;
  localparam LP = 3;
  function int f(); return 7; endfunction
endinterface
module test;
  bus_if b0();
  virtual bus_if v;
  int x;
  initial begin
    v = b0;
    x = v.W;
    x = v.LP;
    $display("FAILED");
  end
endmodule
