// Pass a variable to a task by reference, in the shape of the mailbox
// API.

class mbx_t;
  int slot;

  task get(ref int m);
    m = slot;
  endtask

  function int try_get(ref int m);
    if (slot == 0) return 0;
    m = slot;
    return 1;
  endfunction
endclass

module top;
  initial begin
    mbx_t mb;
    int v;
    mb = new;
    mb.slot = 7;
    v = 0;
    mb.get(v);
    if (v !== 7) begin
      $display("FAILED: v=%0d (expect 7)", v);
      $finish;
    end

    mb.slot = 0;
    if (mb.try_get(v) !== 0 || v !== 7) begin
      $display("FAILED: try_get on empty, v=%0d", v);
      $finish;
    end

    mb.slot = 9;
    if (mb.try_get(v) !== 1 || v !== 9) begin
      $display("FAILED: try_get, v=%0d (expect 9)", v);
      $finish;
    end

    $display("PASSED");
  end
endmodule
