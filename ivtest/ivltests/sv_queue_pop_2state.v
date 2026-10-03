// Check that popping an empty 2-state queue returns zero.

module test;

  bit [7:0] q[$];
  logic [7:0] front, back;

  initial begin
    front = q.pop_front();
    back = q.pop_back();

    if (front !== 8'b0 || back !== 8'b0) begin
      $display("FAILED: pop_front() = %b, pop_back() = %b", front, back);
    end else begin
      $display("PASSED");
    end
  end

endmodule
