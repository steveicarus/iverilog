// Check that the signedness of methods on the built-in enum type is handled
// correctly when calling the function without parenthesis and passing the
// result to a system function.

module test;

  enum shortint {
    A = -1,
    B = -2,
    C = -3
  } es;

  enum bit [15:0] {
    X = 65535,
    Y = 65534,
    Z = 65533
  } eu;

  string s;

  initial begin
    es = B;
    eu = Y;

    s = $sformatf("%0d %0d %0d %0d %0d %0d %0d %0d",
                  es.first, es.last, es.prev, es.next,
                  eu.first, eu.last, eu.prev, eu.next);
    if (s == "-1 -3 -1 -3 65535 65533 65535 65533") begin
      $display("PASSED");
    end else begin
      $display("FAILED s=%s", s);
    end
  end

endmodule
