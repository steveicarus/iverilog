// Check that %p formats real queue elements as real values.

module test;

  real q[$];
  string result;

  initial begin
    q.push_back(1.5);
    result = $sformatf("%p", q);
    if (result == "'{1.5}") begin
      $display("PASSED");
    end else begin
      $display("FAILED: %s", result);
    end
  end

endmodule
