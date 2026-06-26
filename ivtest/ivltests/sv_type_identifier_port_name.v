// Check that module port names can shadow visible type identifiers.

typedef logic [3:0] T;

package p;
  typedef logic [5:0] PT;
endpackage

`define check(value, expected, error) \
  if ((value) !== (expected)) begin \
    $display("FAILED(%0d). %s", `__LINE__, error); \
    $display("  expected %0h, got %0h", expected, value); \
    failed = 1'b1; \
  end

module ansi_name(input T);
endmodule

module ansi_type(input T x);
endmodule

module ansi_name_dim(input T [1:0]);
endmodule

module ansi_type_dim(input T [1:0] x);
endmodule

module ansi_name_list(input T, U);
endmodule

module ansi_type_list(input T x, y);
endmodule

module ansi_type_list_shadow(input T x, T);
endmodule

module ansi_type_name(input T T, output logic ok);
  initial ok = ($bits(T) == 4);
endmodule

module ansi_type_redecl(input n, T x);
endmodule

module ansi_type_redecl_dim(input n, T [1:0] x);
endmodule

module ansi_name_redecl_dim(input n, T [1:0]);
endmodule

module ansi_pkg_type_redecl(input n, p::PT x);
endmodule

module ansi_pkg_type_redecl_dim(input n, p::PT [1:0] x);
endmodule

module decl_name(T);
  input T;
endmodule

module decl_type(x);
  input T x;
endmodule

module decl_type_dim(x);
  input T [1:0] x;
endmodule

module decl_external(.T(x));
  input x;
endmodule

module decl_name_list(T, U);
  input T, U;
endmodule

module decl_type_list(x, y);
  input T x, y;
endmodule

module decl_type_list_shadow(x, T);
  input T x, T;
endmodule

module decl_type_name(T, ok);
  input T T;
  output logic ok;

  initial ok = ($bits(T) == 4);
endmodule

module test;

  reg failed;
  logic n;
  T t;
  logic n_dim [1:0];
  T [1:0] t_dim;
  logic n0, n1;
  T t0, t1;
  logic rn;
  T rt;
  T [1:0] rt_dim;
  logic rn_dim;
  logic rt_name_dim [1:0];
  logic rp_n;
  logic [5:0] rp_t;
  logic rp_dim_n;
  logic [1:0][5:0] rp_t_dim;
  logic d_n;
  T d_t;
  T [1:0] d_t_dim;
  logic d_ext;
  logic d_n0, d_n1;
  T d_t0, d_t1;
  T s_t0, s_t1;
  T d_s_t0, d_s_t1;
  T p_tn;
  T d_p_tn;
  logic p_tn_ok;
  logic d_p_tn_ok;

  ansi_name m0(n);
  ansi_type m1(t);
  ansi_name_dim m2(n_dim);
  ansi_type_dim m3(t_dim);
  ansi_name_list m4(n0, n1);
  ansi_type_list m5(t0, t1);
  ansi_type_list_shadow m17(s_t0, s_t1);
  ansi_type_name m19(p_tn, p_tn_ok);
  ansi_type_redecl m12(rn, rt);
  ansi_type_redecl_dim m13(rn, rt_dim);
  ansi_name_redecl_dim m14(rn_dim, rt_name_dim);
  ansi_pkg_type_redecl m15(rp_n, rp_t);
  ansi_pkg_type_redecl_dim m16(rp_dim_n, rp_t_dim);
  decl_name m6(d_n);
  decl_type m7(d_t);
  decl_type_dim m8(d_t_dim);
  decl_external m9(d_ext);
  decl_name_list m10(d_n0, d_n1);
  decl_type_list m11(d_t0, d_t1);
  decl_type_list_shadow m18(d_s_t0, d_s_t1);
  decl_type_name m20(d_p_tn, d_p_tn_ok);

  initial begin
    failed = 1'b0;

    n = 1'b1;
    t = 4'ha;
    n_dim[0] = 1'b0;
    n_dim[1] = 1'b1;
    t_dim[0] = 4'h3;
    t_dim[1] = 4'hc;
    n0 = 1'b0;
    n1 = 1'b1;
    t0 = 4'h5;
    t1 = 4'ha;
    s_t0 = 4'h6;
    s_t1 = 4'h9;
    p_tn = 4'h3;
    rn = 1'b1;
    rt = 4'hc;
    rt_dim[0] = 4'h3;
    rt_dim[1] = 4'ha;
    rn_dim = 1'b0;
    rt_name_dim[0] = 1'b1;
    rt_name_dim[1] = 1'b0;
    rp_n = 1'b1;
    rp_t = 6'h2a;
    rp_dim_n = 1'b0;
    rp_t_dim[0] = 6'h15;
    rp_t_dim[1] = 6'h2a;
    d_n = 1'b1;
    d_t = 4'h6;
    d_t_dim[0] = 4'h7;
    d_t_dim[1] = 4'h8;
    d_ext = 1'b0;
    d_n0 = 1'b1;
    d_n1 = 1'b0;
    d_t0 = 4'h9;
    d_t1 = 4'h6;
    d_s_t0 = 4'h5;
    d_s_t1 = 4'ha;
    d_p_tn = 4'hc;

    #1;

    `check($bits(n), 1, "input T was not parsed as a port name");
    `check($bits(t), 4, "input T x was not parsed as a typed port");
    `check($bits(n_dim), 2, "input T [1:0] was not parsed as a port array");
    `check($bits(t_dim), 8, "input T [1:0] x was not parsed as a typed port array");
    `check($bits(n0), 1, "input T, U first port did not keep implicit type");
    `check($bits(n1), 1, "input T, U continuation did not keep implicit type");
    `check($bits(t0), 4, "input T x, y first port did not keep typedef type");
    `check($bits(t1), 4, "input T x, y continuation did not keep typedef type");
    `check(t0, 4'h5, "Typed port list first value mismatch");
    `check(t1, 4'ha, "Typed port list continuation mismatch");
    `check($bits(s_t0), 4, "input T x, T first port did not keep typedef type");
    `check($bits(s_t1), 4, "input T x, T continuation did not allow typedef name as port name");
    `check(s_t1, 4'h9, "Typed port list shadowing value mismatch");
    `check($bits(p_tn), 4, "input T T did not keep typedef type");
    `check(p_tn_ok, 1'b1, "input T T was not available as a typed port");
    `check($bits(rn), 1, "input n, T x first port did not keep implicit type");
    `check($bits(rt), 4, "input n, T x second port did not keep typedef type");
    `check(rt, 4'hc, "Inherited-direction typedef port value mismatch");
    `check($bits(rt_dim), 8, "input n, T [1:0] x did not keep typedef packed dimensions");
    `check(rt_dim[0], 4'h3, "Inherited-direction typedef packed array value mismatch");
    `check(rt_dim[1], 4'ha, "Inherited-direction typedef packed array value mismatch");
    `check($bits(rn_dim), 1, "input n, T [1:0] first port did not keep implicit type");
    `check($bits(rt_name_dim), 2, "input n, T [1:0] second port was not parsed as a port array");
    `check($bits(rp_t), 6, "input n, p::PT x second port did not keep package typedef type");
    `check(rp_t, 6'h2a, "Inherited-direction package typedef port value mismatch");
    `check($bits(rp_t_dim), 12, "input n, p::PT [1:0] x did not keep package typedef packed dimensions");
    `check(rp_t_dim[0], 6'h15, "Inherited-direction package typedef packed array value mismatch");
    `check(rp_t_dim[1], 6'h2a, "Inherited-direction package typedef packed array value mismatch");
    `check($bits(d_n), 1, "input T was not parsed as a port declaration name");
    `check($bits(d_t), 4, "input T x was not parsed as a typed port declaration");
    `check($bits(d_t_dim), 8, "input T [1:0] x was not parsed as a typed port declaration array");
    `check($bits(d_ext), 1, ".T(x) was not parsed as an external port name");
    `check($bits(d_n0), 1, "input T, U first declaration did not keep implicit type");
    `check($bits(d_n1), 1, "input T, U continuation declaration did not keep implicit type");
    `check($bits(d_t0), 4, "input T x, y first declaration did not keep typedef type");
    `check($bits(d_t1), 4, "input T x, y continuation declaration did not keep typedef type");
    `check(d_t0, 4'h9, "Typed port declaration list first value mismatch");
    `check(d_t1, 4'h6, "Typed port declaration list continuation mismatch");
    `check($bits(d_s_t0), 4, "input T x, T first declaration did not keep typedef type");
    `check($bits(d_s_t1), 4, "input T x, T declaration did not allow typedef name as port name");
    `check(d_s_t1, 4'ha, "Typed port declaration list shadowing value mismatch");
    `check($bits(d_p_tn), 4, "non-ANSI input T T did not keep typedef type");
    `check(d_p_tn_ok, 1'b1, "non-ANSI input T T was not available as a typed port");

    if (!failed) begin
      $display("PASSED");
    end
  end

endmodule
