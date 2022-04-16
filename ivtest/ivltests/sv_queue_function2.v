// Check that bounded queues are supported as function return types.

module test;

  typedef int Q[$:1];

  function automatic Q f();
    // The last element should be discarded
    return '{1, 2, 3};
  endfunction

  initial begin
    int q[$];

    q = f();

    if (q.size() == 2 && q[0] == 1 && q[1] == 2) begin
      $display("PASSED");
    end else begin
      $display("FAILED");
    end
  end

endmodule
