// Check that foreach resolves static class array properties.

class C;

  static logic [1:0][3:0] values[2][3];

  task count_elements(output integer count);
    count = 0;
    foreach (values[i,j,k,l]) begin
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

    if (count === 48) begin
      $display("PASSED");
    end else begin
      $display("FAILED: Expected 48 elements, got %0d", count);
    end
  end

endmodule
