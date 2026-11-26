// Check that reading an element of an empty queue object returns the
// default value of the element width.

module test;

  logic [7:0] q[$];
  logic [7:0] v;
  int src[$];
  int found[$];
  int found_idx[$];
  int w;
  bit failed = 1'b0;

  initial begin
    // A queue that was emptied by a pop.
    q.push_back(8'h55);
    v = q.pop_back();
    v = q[0];
    if (v !== 8'bx) begin
      $display("FAILED: q[0] = %b", v);
      failed = 1'b1;
    end

    // A queue that was never filled.
    src.push_back(1);
    found = src.find() with (item == 2);
    w = found[0];
    if (w !== 0) begin
      $display("FAILED: found[0] = %0d", w);
      failed = 1'b1;
    end

    // A locator that returns indices has 32 bit elements.
    found_idx = src.find_index() with (item == 2);
    w = found_idx[0];
    if (w !== 0) begin
      $display("FAILED: found_index[0] = %0d", w);
      failed = 1'b1;
    end

    if (!failed) $display("PASSED");
  end

endmodule
