// A const ref argument can be read, and sees the changes made to the
// variable while the task runs.

module top;
  int v;

  function automatic int get(const ref int x);
    return x + 1;
  endfunction

  task automatic watch(const ref int x, output int seen1, output int seen2);
    seen1 = x;
    #10;
    seen2 = x;
  endtask

  initial begin
    int a, b;
    v = 3;
    if (get(v) !== 4) begin
      $display("FAILED: get(v)=%0d (expect 4)", get(v));
      $finish;
    end

    fork
      watch(v, a, b);
      begin
        #5;
        v = 8;
      end
    join

    if (a !== 3 || b !== 8) begin
      $display("FAILED: a=%0d b=%0d (expect 3, 8)", a, b);
      $finish;
    end
    $display("PASSED");
  end
endmodule
