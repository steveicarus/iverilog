// Check that nested array method calls can each have a with clause.

module test;
  int data[$], result[$];

  initial begin
    data.push_back(1);
    data.push_back(2);
    data.push_back(3);

    result = data.find() with (item == (data.sum() with (item * 2) - 10));
    if (result.size() == 1 && result[0] === 2) begin
      $display("PASSED");
    end else begin
      $display("FAILED: expected '{2}, got %p", result);
    end
  end
endmodule
