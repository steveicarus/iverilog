// Check assignment operations in constant functions.
module constfunc7();

function real i_to_r(input signed [3:0] value);
  i_to_r = value + 0.5;
endfunction

function signed [3:0] r_to_i(input real value);
  r_to_i = value;
endfunction

function real u_to_r(input [3:0] value);
  u_to_r = value + 0.5;
endfunction

function [3:0] r_to_u(input real value);
  r_to_u = value;
endfunction

function [3:0] i_to_u(input signed [3:0] value);
  i_to_u = value;
endfunction

function signed [3:0] u_to_i(input [3:0] value);
  u_to_i = value;
endfunction

function [5:0] si_to_lu(input signed [3:0] value);
  si_to_lu = value;
endfunction

function signed [5:0] su_to_li(input [3:0] value);
  su_to_li = value;
endfunction

function [3:0] li_to_su(input signed [5:0] value);
  li_to_su = value;
endfunction

function signed [3:0] lu_to_si(input [5:0] value);
  lu_to_si = value;
endfunction

localparam i_to_r_res1 = i_to_r(-9);
localparam i_to_r_res2 = i_to_r(-8);
localparam i_to_r_res3 = i_to_r( 7);
localparam i_to_r_res4 = i_to_r( 8);

localparam r_to_i_res1 = r_to_i(-8.5);
localparam r_to_i_res2 = r_to_i(-7.5);
localparam r_to_i_res3 = r_to_i( 6.5);
localparam r_to_i_res4 = r_to_i( 7.5);

localparam u_to_r_res1 = u_to_r(-1);
localparam u_to_r_res2 = u_to_r( 1);
localparam u_to_r_res3 = u_to_r(15);
localparam u_to_r_res4 = u_to_r(17);

localparam r_to_u_res1 = r_to_u(-0.5);
localparam r_to_u_res2 = r_to_u( 0.5);
localparam r_to_u_res3 = r_to_u(14.5);
localparam r_to_u_res4 = r_to_u(16.5);

localparam i_to_u_res1 = i_to_u(-9);
localparam i_to_u_res2 = i_to_u(-8);
localparam i_to_u_res3 = i_to_u( 7);
localparam i_to_u_res4 = i_to_u( 8);

localparam u_to_i_res1 = u_to_i(-1);
localparam u_to_i_res2 = u_to_i( 1);
localparam u_to_i_res3 = u_to_i(15);
localparam u_to_i_res4 = u_to_i(17);

localparam si_to_lu_res1 = si_to_lu(-9);
localparam si_to_lu_res2 = si_to_lu(-8);
localparam si_to_lu_res3 = si_to_lu( 7);
localparam si_to_lu_res4 = si_to_lu( 8);

localparam su_to_li_res1 = su_to_li(-1);
localparam su_to_li_res2 = su_to_li( 1);
localparam su_to_li_res3 = su_to_li(15);
localparam su_to_li_res4 = su_to_li(17);

localparam li_to_su_res1 = li_to_su(-9);
localparam li_to_su_res2 = li_to_su(-8);
localparam li_to_su_res3 = li_to_su( 7);
localparam li_to_su_res4 = li_to_su( 8);

localparam lu_to_si_res1 = lu_to_si(-1);
localparam lu_to_si_res2 = lu_to_si( 1);
localparam lu_to_si_res3 = lu_to_si(15);
localparam lu_to_si_res4 = lu_to_si(17);

reg failed;

initial begin
  failed = 0;

  $display("%0g", i_to_r_res1); if (i_to_r_res1 !=  7.5) failed = 1;
  $display("%0g", i_to_r_res2); if (i_to_r_res2 != -7.5) failed = 1;
  $display("%0g", i_to_r_res3); if (i_to_r_res3 !=  7.5) failed = 1;
  $display("%0g", i_to_r_res4); if (i_to_r_res4 != -7.5) failed = 1;
  $display("");
  $display("%0d", r_to_i_res1); if (r_to_i_res1 !==  7) failed = 1;
  $display("%0d", r_to_i_res2); if (r_to_i_res2 !== -8) failed = 1;
  $display("%0d", r_to_i_res3); if (r_to_i_res3 !==  7) failed = 1;
  $display("%0d", r_to_i_res4); if (r_to_i_res4 !== -8) failed = 1;
  $display("");
  $display("%0g", u_to_r_res1); if (u_to_r_res1 != 15.5) failed = 1;
  $display("%0g", u_to_r_res2); if (u_to_r_res2 !=  1.5) failed = 1;
  $display("%0g", u_to_r_res3); if (u_to_r_res3 != 15.5) failed = 1;
  $display("%0g", u_to_r_res4); if (u_to_r_res4 !=  1.5) failed = 1;
  $display("");
  $display("%0d", r_to_u_res1); if (r_to_u_res1 !== 15) failed = 1;
  $display("%0d", r_to_u_res2); if (r_to_u_res2 !==  1) failed = 1;
  $display("%0d", r_to_u_res3); if (r_to_u_res3 !== 15) failed = 1;
  $display("%0d", r_to_u_res4); if (r_to_u_res4 !==  1) failed = 1;
  $display("");
  $display("%0d", i_to_u_res1); if (i_to_u_res1 !==  7) failed = 1;
  $display("%0d", i_to_u_res2); if (i_to_u_res2 !==  8) failed = 1;
  $display("%0d", i_to_u_res3); if (i_to_u_res3 !==  7) failed = 1;
  $display("%0d", i_to_u_res4); if (i_to_u_res4 !==  8) failed = 1;
  $display("");
  $display("%0d", u_to_i_res1); if (u_to_i_res1 !== -1) failed = 1;
  $display("%0d", u_to_i_res2); if (u_to_i_res2 !==  1) failed = 1;
  $display("%0d", u_to_i_res3); if (u_to_i_res3 !== -1) failed = 1;
  $display("%0d", u_to_i_res4); if (u_to_i_res4 !==  1) failed = 1;
  $display("");
  $display("%0d", si_to_lu_res1); if (si_to_lu_res1 !==  7) failed = 1;
  $display("%0d", si_to_lu_res2); if (si_to_lu_res2 !== 56) failed = 1;
  $display("%0d", si_to_lu_res3); if (si_to_lu_res3 !==  7) failed = 1;
  $display("%0d", si_to_lu_res4); if (si_to_lu_res4 !== 56) failed = 1;
  $display("");
  $display("%0d", su_to_li_res1); if (su_to_li_res1 !== 15) failed = 1;
  $display("%0d", su_to_li_res2); if (su_to_li_res2 !==  1) failed = 1;
  $display("%0d", su_to_li_res3); if (su_to_li_res3 !== 15) failed = 1;
  $display("%0d", su_to_li_res4); if (su_to_li_res4 !==  1) failed = 1;
  $display("");
  $display("%0d", li_to_su_res1); if (li_to_su_res1 !==  7) failed = 1;
  $display("%0d", li_to_su_res2); if (li_to_su_res2 !==  8) failed = 1;
  $display("%0d", li_to_su_res3); if (li_to_su_res3 !==  7) failed = 1;
  $display("%0d", li_to_su_res4); if (li_to_su_res4 !==  8) failed = 1;
  $display("");
  $display("%0d", lu_to_si_res1); if (lu_to_si_res1 !== -1) failed = 1;
  $display("%0d", lu_to_si_res2); if (lu_to_si_res2 !==  1) failed = 1;
  $display("%0d", lu_to_si_res3); if (lu_to_si_res3 !== -1) failed = 1;
  $display("%0d", lu_to_si_res4); if (lu_to_si_res4 !==  1) failed = 1;
  $display("");
  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
