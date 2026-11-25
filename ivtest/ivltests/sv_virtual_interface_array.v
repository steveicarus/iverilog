// Check that a virtual interface selects the right instance among instance
// arrays, generated instances and instances inside other modules, and that
// handles stored in a class do not go stale when they are assigned again.

interface bus_if;
  logic [7:0] data = 0;
endinterface

module sub;
  bus_if bi();
endmodule

class holder;
  local virtual bus_if vif;
  function void set(virtual bus_if v);
    vif = v;
  endfunction
  function [7:0] get();
    return vif.data;
  endfunction
  task put(input [7:0] d);
    vif.data <= d;
  endtask
endclass

module test;
  bus_if b[2:0]();
  sub u0();
  sub u1();
  genvar g;
  generate
    for (g = 0; g < 2; g = g + 1) begin : gen
      bus_if bg();
    end
  endgenerate

  virtual bus_if v[7];
  virtual bus_if cur;
  holder h[7];
  holder t;
  int fails = 0;

  initial begin
    v[0] = b[0]; v[1] = b[1]; v[2] = b[2];
    v[3] = u0.bi; v[4] = u1.bi;
    v[5] = gen[0].bg; v[6] = gen[1].bg;

    // Each handle is distinct and writes reach one instance only.
    for (int i = 0; i < 7; i++) begin
      for (int j = 0; j < 7; j++)
        if ((v[i] == v[j]) != (i == j)) begin
          $display("FAILED: compare %0d %0d", i, j);
          fails++;
        end
      cur = v[i];
      cur.data = 8'h10 + i;
    end
    if (b[0].data !== 8'h10 || b[1].data !== 8'h11 || b[2].data !== 8'h12 ||
        u0.bi.data !== 8'h13 || u1.bi.data !== 8'h14 ||
        gen[0].bg.data !== 8'h15 || gen[1].bg.data !== 8'h16) begin
      $display("FAILED: wrong instance written");
      fails++;
    end

    // Reads through handles held by objects.
    for (int i = 0; i < 7; i++) begin
      t = new;
      t.set(v[i]);
      h[i] = t;
    end
    for (int i = 0; i < 7; i++) begin
      t = h[i];
      if (t.get() !== 8'h10 + i) begin
        $display("FAILED: wrong instance read %0d: %h", i, t.get());
        fails++;
      end
    end

    // Assign the handles again in reverse order. The old instances must
    // not be touched.
    for (int i = 0; i < 7; i++) begin
      t = h[i];
      t.set(v[6-i]);
    end
    for (int i = 0; i < 7; i++) begin
      t = h[i];
      t.put(8'h20 + i);
    end
    #1;
    for (int i = 0; i < 7; i++) begin
      cur = v[6-i];
      t = h[i];
      if (cur.data !== 8'h20 + i || t.get() !== 8'h20 + i) begin
        $display("FAILED: stale handle %0d: %h", i, cur.data);
        fails++;
      end
    end
    if (b[0].data !== 8'h26 || gen[1].bg.data !== 8'h20 || u0.bi.data !== 8'h23) begin
      $display("FAILED: stale handle");
      fails++;
    end

    if (fails == 0) $display("PASSED");
  end
endmodule
