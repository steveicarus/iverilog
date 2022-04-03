// Check that the return value of a queue pop method is sign extended correctly.

module test;

  shortint qs[$];
  int x, y;

  bit [15:0] qu[$];
  int w, z;

  initial begin
    qs.push_back(-1);
    qs.push_back(-2);
    qu.push_back(-1);
    qu.push_back(-2);

    // The returned value is signed and should be sign extended
    x = qs.pop_front();
    y = qs.pop_back();
    // The returned value is unsigned and should not be sign extended
    w = qu.pop_front();
    z = qu.pop_back();

    if (x === -1 && y === -2 && w == 65535 && z == 65534) begin
      $display("PASSED");
    end else begin
      $display("FAILED x=%0d, y=%0d, z=%0d, w=%0d", x, y, z, w);
    end
  end

endmodule
