// Check that a virtual interface cannot be a member of a union
// (IEEE1800-2017 25.9).
interface bus_if;
  logic [7:0] data;
endinterface

typedef union packed {
  virtual bus_if v;
  logic [63:0] x;
} u_t;

module test;
  bus_if b0();
  u_t u;
  initial $display("FAILED");
endmodule
