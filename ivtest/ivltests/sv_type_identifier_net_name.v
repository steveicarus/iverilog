// Check that net names can shadow visible type identifiers.

typedef logic [7:0] T;
typedef int U;
typedef logic [5:0] V;

`define check(value, expected, error) \
  if ((value) !== (expected)) begin \
    $display("FAILED(%0d). %s", `__LINE__, error); \
    $display("  expected %0h, got %0h", expected, value); \
    failed = 1'b1; \
  end

module net_name(output reg failed);
  wire T = 1'b1;

  initial begin
    failed = 1'b0;

    `check(T, 1'b1, "wire T did not declare a one-bit net");
  end
endmodule

module net_list(output reg failed);
  wire T, U;

  assign T = 1'b0;
  assign U = 1'b1;

  initial begin
    failed = 1'b0;

    `check(T, 1'b0, "type identifier net list first value mismatch");
    `check(U, 1'b1, "type identifier net list continuation mismatch");
  end
endmodule

module net_array(output reg failed);
  wire T [1:0];

  assign T[0] = 1'b0;
  assign T[1] = 1'b1;

  initial begin
    failed = 1'b0;

    `check(T[0], 1'b0, "type identifier net array first value mismatch");
    `check(T[1], 1'b1, "type identifier net array second value mismatch");
  end
endmodule

module net_type(output reg failed);
  wire T x = 8'ha5;
  wire T [1:0] y = 16'h5aa5;
  wire V v0, V;
  wire T T;

  assign v0 = 6'h2a;
  assign V = 6'h15;
  assign T = 8'h3c;

  initial begin
    failed = 1'b0;

    `check($bits(x), 8, "typed net declaration width regressed");
    `check(x, 8'ha5, "typed net declaration value regressed");
    `check($bits(y), 16, "typed packed net declaration width regressed");
    `check(y, 16'h5aa5, "typed packed net declaration value regressed");
    `check($bits(v0), 6, "type identifier net declaration list first width mismatch");
    `check($bits(V), 6, "type identifier net declaration list did not allow typedef name as continuation");
    `check(V, 6'h15, "type identifier net declaration list value mismatch");
    `check($bits(T), 8, "type-name net declaration did not keep typedef type");
    `check(T, 8'h3c, "type-name net declaration value mismatch");
  end
endmodule

module test;
  reg failed;
  wire f0;
  wire f1;
  wire f2;
  wire f3;

  net_name m0(f0);
  net_list m1(f1);
  net_array m2(f2);
  net_type m3(f3);

  initial begin
    failed = 1'b0;

    #1;

    `check(f0, 1'b0, "wire T module failed");
    `check(f1, 1'b0, "wire T, U module failed");
    `check(f2, 1'b0, "wire T array module failed");
    `check(f3, 1'b0, "typed wire T module failed");

    if (!failed) begin
      $display("PASSED");
    end
  end
endmodule
