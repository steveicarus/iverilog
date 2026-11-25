// Check that a path below a member of a virtual interface gives a sorry.
typedef struct packed {
  logic [3:0] x;
  logic [3:0] y;
} pair_t;

interface bus_if;
  pair_t data;
endinterface

module test;
  bus_if b0();
  virtual bus_if v;
  logic [3:0] x;
  initial begin
    v = b0;
    v.data.x = 4'h1;
    x = v.data.y;
    $display("FAILED");
  end
endmodule
