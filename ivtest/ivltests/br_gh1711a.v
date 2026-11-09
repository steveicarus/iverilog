// Check that %p formats string queue elements as strings.

module test;

  string q[$];
  string result;

  initial begin
    q.push_back("abc");
    result = $sformatf("%p", q);
    if (result == "'{\"abc\"}") begin
      $display("PASSED");
    end else begin
      $display("FAILED: %s", result);
    end
  end

endmodule
