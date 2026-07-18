module test;
  logic a;

  always_comb begin
    my_blk: begin
      a = 1'b1;
    end
  end

  initial begin
    #1;
    if (a !== 1'b1) begin
      $display("FAILED(%0d). Expected 1, got %b", `__LINE__, a);
    end else begin
      $display("PASSED");
    end
  end
endmodule
