// Check that the signedness of the element type of a queue is correctly handled
// when passing the result of one of the pop methods as an argument to a system
// function.

module test;

  shortint qs[$];
  bit [15:0] qu[$];

  string s;

  initial begin
    qs.push_back(-1);
    qs.push_back(-2);
    qu.push_back(-1);
    qu.push_back(-2);

    // Values popped from qs should be treated as signed, values popped from qu
    // should be treated as unsigned
    s = $sformatf("%0d %0d %0d %0d", qs.pop_front, qs.pop_back,
                                     qu.pop_front, qu.pop_back);
    if (s == "-1 -2 65535 65534") begin
      $display("PASSED");
    end else begin
      $display("FAILED s=%s", s);
    end
  end

endmodule
