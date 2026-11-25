// Check that a virtual interface type can be given as the value of a
// type parameter of a class or a module.
interface bus_if;
  logic [7:0] data = 0;
endinterface

class holder #(type T = int);
  T v;
endclass

class holder_vif #(type T = virtual bus_if);
  T v;
endclass

class cdb #(type T = int);
  static T slot;
  static function void set(T v);
    slot = v;
  endfunction
  static function T get();
    return slot;
  endfunction
endclass

module sub #(parameter type T = int);
  T v;
endmodule

module test;
  bus_if b0();
  bus_if b1();
  bus_if b2();
  bus_if b3();
  bus_if b4();

  holder #(virtual bus_if) h1;
  holder #(virtual interface bus_if) h2;
  holder #(.T(virtual bus_if)) h3;
  holder_vif h4;
  sub #(virtual bus_if) s0();
  virtual bus_if v;
  bit failed = 0;

  initial begin
    h1 = new;
    h1.v = b0;
    h1.v.data = 8'h11;
    h2 = new;
    h2.v = b1;
    h2.v.data = 8'h22;
    h3 = new;
    h3.v = b2;
    h3.v.data = 8'h33;
    h4 = new;
    h4.v = b3;
    h4.v.data = 8'h44;
    s0.v = b4;
    s0.v.data = 8'h55;
    cdb#(virtual bus_if)::set(b2);
    v = cdb#(virtual interface bus_if)::get();

    if (b0.data !== 8'h11) failed = 1;
    if (b1.data !== 8'h22) failed = 1;
    if (b2.data !== 8'h33) failed = 1;
    if (b3.data !== 8'h44) failed = 1;
    if (b4.data !== 8'h55) failed = 1;
    if (v != b2 || v.data !== 8'h33) failed = 1;
    if (h1.v.data !== 8'h11) failed = 1;

    if (failed)
      $display("FAILED");
    else
      $display("PASSED");
  end
endmodule
