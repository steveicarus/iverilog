// Check that the words of variable arrays report vpiVector when the
// element has a packed range (even a single-element one) or is wider than
// one bit, and vpiScalar for plain scalar elements.

module test;

  bit               b_s [1:0];
  bit               b_s2 [1:0][1:0];
  bit [0:0]         b_v [1:0];
  bit signed        bs_s [1:0];
  bit signed [0:0]  bs_v [1:0];
  logic signed      ls_s [1:0];
  logic signed [0:0] ls_v [1:0];
  logic [1:0]       l2 [1:0];
  logic [0:0]       l_v [1:0];
  logic             l_s [1:0];
  bit [7:0]         b8 [1:0];
  bit signed [7:0]  b8s [1:0];
  int               int_arr [1:0];
  integer           i_arr [1:0];
  bit [0:0]         v_down [1:0];
  bit [0:0]         v_up [0:1];
  wire [0:0]        n_v [1:0];
  wire              n_s [1:0];
  assign n_v[0] = 1'b0;
  assign n_v[1] = 1'b0;
  assign n_s[0] = 1'b0;
  assign n_s[1] = 1'b0;
  integer           idx;
  integer           failed;

  task automatic auto_check;
    bit [0:0] a_v [1:0];
    bit       a_s [1:0];
    begin
      failed |= $check_sv(a_v[0], 1, 1);
      failed |= $check_sv(a_s[0], 0, 1);
    end
  endtask

  initial begin
    failed = 0;
    idx = 1;
    // Constant word select.
    failed |= $check_sv(b_s[0], 0, 1);
    failed |= $check_sv(b_s2[1][0], 0, 1);
    failed |= $check_sv(b_v[1], 1, 1);
    failed |= $check_sv(bs_s[0], 0, 1);
    failed |= $check_sv(bs_v[1], 1, 1);
    failed |= $check_sv(ls_s[0], 0, 1);
    failed |= $check_sv(ls_v[1], 1, 1);
    failed |= $check_sv(l_s[0], 0, 1);
    failed |= $check_sv(l_v[1], 1, 1);
    failed |= $check_sv(l2[0], 1, 2);
    failed |= $check_sv(b8[0], 1, 8);
    failed |= $check_sv(b8s[0], 1, 8);
    failed |= $check_sv(int_arr[1], 1, 32);
    failed |= $check_sv(i_arr[0], 1, 32);
    // Net arrays and automatic arrays.
    failed |= $check_sv(n_v[0], 1, 1);
    failed |= $check_sv(n_s[1], 0, 1);
    auto_check;
    // Variable word select.
    failed |= $check_sv(b_s[idx], 0, 1);
    failed |= $check_sv(b_v[idx], 1, 1);
    failed |= $check_sv(l_s[idx], 0, 1);
    failed |= $check_sv(l_v[idx], 1, 1);
    failed |= $check_sv(l2[idx], 1, 2);
    // Word iteration.
    failed |= $check_sv_words(b_s, 0, 1);
    failed |= $check_sv_words(b_v, 1, 1);
    failed |= $check_sv_words(bs_s, 0, 1);
    failed |= $check_sv_words(bs_v, 1, 1);
    failed |= $check_sv_words(ls_s, 0, 1);
    failed |= $check_sv_words(ls_v, 1, 1);
    failed |= $check_sv_words(l_s, 0, 1);
    failed |= $check_sv_words(l_v, 1, 1);
    failed |= $check_sv_words(l2, 1, 2);
    failed |= $check_sv_words(b8s, 1, 8);
    failed |= $check_sv_words(int_arr, 1, 32);
    failed |= $check_sv_words(v_down, 1, 1);
    failed |= $check_sv_words(v_up, 1, 1);
    if (!failed) $display("PASSED");
  end

endmodule
