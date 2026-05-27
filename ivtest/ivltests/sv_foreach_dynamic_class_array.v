// Check that foreach resolves dynamic class array properties.

class C;

  integer values[];

  task count_elements(output integer count);
    values = new[3];
    count = 0;
    foreach (values[i]) begin
      count = count + 1;
    end
  endtask

endclass

module test;

  C c;
  integer count;

  initial begin
    c = new;
    c.count_elements(count);

    if (count === 3) begin
      $display("PASSED");
    end else begin
      $display("FAILED: Expected 3 elements, got %0d", count);
    end
  end

endmodule
