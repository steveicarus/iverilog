// Check that a package-qualified delay value does not require parentheses.

package p;
  parameter D = 2;
endpackage

module test;

  initial begin
    #p::D;
    if ($time != 2) begin
      $display("FAILED: time=%0t", $time);
    end else begin
      $display("PASSED");
    end
  end

endmodule
