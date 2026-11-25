// Check that a virtual interface cannot be an item of an interface
// (IEEE1800-2017 25.9).
interface bus_if;
  logic [7:0] data;
endinterface

interface wrap_if;
  virtual bus_if v;
endinterface

module test;
  wrap_if w0();
  initial $display("FAILED");
endmodule
