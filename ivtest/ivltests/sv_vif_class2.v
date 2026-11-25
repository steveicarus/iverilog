// class property, constructor/task/function arguments
interface bus_if;
  logic [7:0] data;
  logic       clk;
endinterface

class driver;
  virtual bus_if vif;
  local virtual interface bus_if lvif;

  function new(virtual bus_if v);
    vif = v;
    lvif = v;
  endfunction

  task drive(logic [7:0] d);
    vif.data = d;
  endtask

  task drive_nb(logic [7:0] d);
    vif.data <= d;
  endtask

  function logic [7:0] sample();
    return vif.data;
  endfunction

  function logic [7:0] lsample();
    return lvif.data;
  endfunction

  function void set(virtual bus_if v);
    vif = v;
  endfunction

  function bit is_null();
    return vif == null;
  endfunction
endclass

module test;
  bus_if b1();
  bus_if b2();
  driver d;
  virtual bus_if m;
  bit pass = 1;

  task automatic poke(virtual bus_if v, logic [7:0] x);
    v.data = x;
  endtask

  function automatic logic [7:0] peek(virtual bus_if v);
    return v.data;
  endfunction

  initial begin
    b1.data = 0;
    b2.data = 0;
    d = new(b1);
    d.drive(8'hA1);
    if (b1.data !== 8'hA1 || b2.data !== 8'h00) begin $display("FAILED: drive b1 %h %h", b1.data, b2.data); pass = 0; end
    d.set(b2);
    d.drive(8'hB2);
    if (b1.data !== 8'hA1 || b2.data !== 8'hB2) begin $display("FAILED: drive b2 %h %h", b1.data, b2.data); pass = 0; end
    if (d.sample() !== 8'hB2) begin $display("FAILED: sample"); pass = 0; end
    if (d.lsample() !== 8'hA1) begin $display("FAILED: lsample"); pass = 0; end
    d.drive_nb(8'hC3);
    #1;
    if (b2.data !== 8'hC3) begin $display("FAILED: drive_nb %h", b2.data); pass = 0; end
    m = d.vif;
    if (m != b2) begin $display("FAILED: copy from property"); pass = 0; end
    d.vif = b1;
    if (d.vif != b1) begin $display("FAILED: property assign"); pass = 0; end
    if (d.vif.data !== 8'hA1) begin $display("FAILED: d.vif.data %h", d.vif.data); pass = 0; end
    d.vif.data = 8'hD4;
    if (b1.data !== 8'hD4) begin $display("FAILED: d.vif.data write %h", b1.data); pass = 0; end
    if (d.is_null()) begin $display("FAILED: is_null"); pass = 0; end
    d.set(null);
    if (!d.is_null()) begin $display("FAILED: is_null after null"); pass = 0; end
    poke(b2, 8'hE5);
    if (b2.data !== 8'hE5 || b1.data !== 8'hD4) begin $display("FAILED: poke %h %h", b1.data, b2.data); pass = 0; end
    if (peek(b1) !== 8'hD4) begin $display("FAILED: peek b1"); pass = 0; end
    m = b1;
    if (peek(m) !== 8'hD4) begin $display("FAILED: peek m"); pass = 0; end
    if (pass) $display("PASSED");
  end
endmodule
