// Check that void casts on methods of built-in types is supported

module test;

  int q[$];

  initial begin
    q.push_back(1);
    void'(q.pop_back());

    if (q.size() === 0) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
