// Check that variable names can shadow visible type identifiers.

typedef logic [7:0] T;
typedef int V;
typedef int W;
typedef logic [6:0] A;
typedef logic [5:0] B;
typedef logic [4:0] C;
typedef logic [3:0] D;
typedef logic [2:0] E;
typedef logic [1:0] F;
typedef logic [2:0] G;
typedef logic [3:0] H;
typedef logic [4:0] I;
typedef logic [2:0] P;
bit X;

package p;
  var C;
  var D x, D;
  var P P;
endpackage

module test;

  reg failed;

  `define check(value, expected, error) \
    if ((value) !== (expected)) begin \
      $display("FAILED(%0d). %s", `__LINE__, error); \
      $display("  expected %0h, got %0h", expected, value); \
      failed = 1'b1; \
    end

  typedef logic [3:0] X;

  T outer;
  T a, b;
  A a0, A;
  E E;
  int T, U;
  var V;
  var B b0, B;
  var F F;
  X x;

  function int f;
    int T;

    T = 32'd11;
    return T;
  endfunction

  function int f_type_name;
    H H;

    H = 4'ha;
    return $bits(H) + H;
  endfunction

  task t(output int value);
    int T;

    T = 32'd22;
    value = T;
  endtask

  task t_type_name(output int value);
    I I;

    I = 5'h15;
    value = $bits(I) + I;
  endtask

  initial begin
    int r;
    int r_type_name;
    int tr_type_name;
    var W;

    failed = 1'b0;

    outer = 8'ha5;
    a = 8'h33;
    b = 8'hcc;
    a0 = 7'h2a;
    A = 7'h15;
    E = 3'h5;
    T = 32'd23;
    U = 32'd41;
    V = 1'b1;
    b0 = 6'h2a;
    B = 6'h15;
    F = 2'h2;
    W = 1'b0;
    x = 4'hc;
    t(r);
    t_type_name(tr_type_name);

    begin : block_scope
      int T;

      T = 32'd7;
      `check(T, 32'd7, "Block declaration did not hide typedef");
    end

    begin : block_type_name
      G G;

      G = 3'h3;
      `check($bits(G), 3, "Block type-name declaration did not keep typedef type");
      `check(G, 3'h3, "Block type-name declaration value mismatch");
    end

    r_type_name = f_type_name();

    `check(outer, 8'ha5, "Typedef value changed");
    `check(T, 32'd23, "Module declaration did not hide typedef");
    `check(U, 32'd41, "Declaration list continuation mismatch");
    `check($bits(V), 1, "Module var declaration did not hide typedef width");
    `check(V, 1'b1, "Module var declaration did not hide typedef value");
    `check($bits(W), 1, "Block var declaration did not hide typedef width");
    `check(W, 1'b0, "Block var declaration did not hide typedef value");
    `check(a, 8'h33, "Type declaration list first value mismatch");
    `check(b, 8'hcc, "Type declaration list continuation mismatch");
    `check($bits(a0), 7, "Type declaration list did not keep typedef type");
    `check($bits(A), 7, "Type declaration list did not allow typedef name as continuation");
    `check(A, 7'h15, "Type declaration list shadowing value mismatch");
    `check($bits(E), 3, "Type-name declaration did not keep typedef type");
    `check(E, 3'h5, "Type-name declaration value mismatch");
    `check($bits(b0), 6, "Var declaration list did not keep typedef type");
    `check($bits(B), 6, "Var declaration list did not allow typedef name as continuation");
    `check(B, 6'h15, "Var declaration list shadowing value mismatch");
    `check($bits(F), 2, "Var type-name declaration did not keep typedef type");
    `check(F, 2'h2, "Var type-name declaration value mismatch");
    `check($bits(p::C), 1, "Package var declaration did not hide typedef width");
    `check($bits(p::x), 4, "Package type declaration list first width mismatch");
    `check($bits(p::D), 4, "Package type declaration list did not allow typedef name as continuation");
    `check($bits(p::P), 3, "Package type-name declaration did not keep typedef type");
    `check(f(), 32'd11, "Function declaration did not hide typedef");
    `check(r_type_name, 14, "Function type-name declaration mismatch");
    `check(r, 32'd22, "Task declaration did not hide typedef");
    `check(tr_type_name, 26, "Task type-name declaration mismatch");
    `check($bits(x), 4, "Local typedef did not hide outer identifier");
    `check(x, 4'hc, "Local typedef value mismatch");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
