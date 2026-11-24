// compile errors: wrong interface type, unknown member
interface bus_if;
  logic [7:0] data;
endinterface

interface other_if;
  logic [7:0] data;
endinterface

module test;
  bus_if b1();
  other_if o1();
  virtual bus_if v;
  virtual other_if ov;

  initial begin
    v = o1;
    v = ov;
    v.nosuch = 1;
  end
endmodule
