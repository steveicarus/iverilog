// Check that $attribute targets can match visible type identifiers.

primitive ATTR_TYPE (out, in);
  output out;
  input in;

  table
    0 : 0;
    1 : 1;
  endtable
endprimitive

typedef int ATTR_TYPE;

$attribute(ATTR_TYPE, "test", "true")

module test;

  initial begin
    $display("PASSED");
  end

endmodule
