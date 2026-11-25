// module variable: assign, copy, compare, read, blocking and non-blocking write
interface bus_if;
  logic [7:0] data;
  logic       clk;
endinterface

module test;
  bus_if b1();
  bus_if b2();

  virtual bus_if v;
  virtual interface bus_if w;
  logic [7:0] r;
  bit pass = 1;

  initial begin
    b1.data = 8'h11;
    b2.data = 8'h22;
    if (v != null) begin $display("FAILED: not null at start"); pass = 0; end
    if (!(v == null)) begin $display("FAILED: == null"); pass = 0; end
    v = b1;
    if (v == null) begin $display("FAILED: null after assign"); pass = 0; end
    if (v != b1) begin $display("FAILED: v != b1"); pass = 0; end
    if (v == b2) begin $display("FAILED: v == b2"); pass = 0; end
    r = v.data;
    if (r !== 8'h11) begin $display("FAILED: read b1 %h", r); pass = 0; end
    v = b2;
    r = v.data;
    if (r !== 8'h22) begin $display("FAILED: read b2 %h", r); pass = 0; end
    w = v;
    if (w != b2) begin $display("FAILED: copy"); pass = 0; end
    if (w != v) begin $display("FAILED: w != v"); pass = 0; end
    v = b1;
    if (w != b2) begin $display("FAILED: stale copy"); pass = 0; end
    v = b2;
    w.data = 8'h33;
    if (b2.data !== 8'h33 || b1.data !== 8'h11) begin $display("FAILED: blocking write %h %h", b1.data, b2.data); pass = 0; end
    w = b1;
    w.data <= 8'h44;
    #1;
    if (b1.data !== 8'h44 || b2.data !== 8'h33) begin $display("FAILED: nb write %h %h", b1.data, b2.data); pass = 0; end
    w.data[3:0] = 4'h5;
    if (b1.data !== 8'h45) begin $display("FAILED: part select write %h", b1.data); pass = 0; end
    w.data += 1;
    if (b1.data !== 8'h46 || b2.data !== 8'h33) begin $display("FAILED: += %h %h", b1.data, b2.data); pass = 0; end
    if (v.data[3:0] !== 4'h3) begin $display("FAILED: part select read"); pass = 0; end
    if (v.data + w.data !== 8'h79) begin $display("FAILED: expr %h", v.data + w.data); pass = 0; end
    w = null;
    if (w != null) begin $display("FAILED: set null"); pass = 0; end
    if (pass) $display("PASSED");
  end
endmodule
