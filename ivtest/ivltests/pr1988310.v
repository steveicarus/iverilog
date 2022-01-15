module top;
  localparam A = 0;
  reg pass = 1'b1;

  generate
    if (A < 1) begin: gen
      task foo_task;
        reg x;
        begin
          x = 1'b0;
          #10;
          x = 1'b1;
        end
      endtask
    end else begin: gen
      task foo_task;
        reg x;
        begin
          x = 1'b1;
          #10;
          x = 1'b0;
        end
      endtask
  end
  endgenerate

  initial begin
    gen.foo_task;
  end

  initial begin
    #9
    if (gen.foo_task.x !== 1'b0) begin
      $display("Failed: expected 1'b0, got %b", gen.foo_task.x);
      pass = 1'b0;
    end

    #2
    if (gen.foo_task.x !== 1'b1) begin
      $display("Failed: expected 1'b1, got %b", gen.foo_task.x);
      pass = 1'b0;
    end

    if (pass) $display("PASSED");
  end
endmodule
