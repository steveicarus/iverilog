// sorry: writing a member through an element of an array, queue or
// dynamic array of virtual interfaces that is held in a class
interface bus_if;
  logic [7:0] data;
endinterface

class env;
  virtual bus_if va[2];
  virtual bus_if vq[$];
  virtual bus_if vd[];
  task put(int k);
    va[k].data = 1;
    vq[k].data = 2;
    vd[k].data = 3;
  endtask
endclass

module test;
  bus_if b0();
  env e;
  initial begin
    e = new;
    e.va[1].data = 4;
  end
endmodule
