`timescale 1ns/1ps

primitive not_u (out, in);
  output out;
  input in;
  table
    0 : 1;
    1 : 0;
  endtable
endprimitive

// Any instance of this gate will use the small time scale that was
// in place when it was defined.
module gate_sdf(out, in);
  output out;
  input in;
  wire out, in;

  assign out = ~in;

  specify
    (in => out) = (0.5, 0.5);
  endspecify
endmodule

/*
 * Icarus does not currently support UDPs with a variable delay.
 * It also needs to support NULL decay delays, buf/not/etc. should
 * only allow two delays maximum.
 */
module top;
  initial begin
    $monitor("%.3f", $realtime,, sml_const.test, sml_var.test,
                                 sml_const.out_g, sml_const.out_u,
                                 sml_const.out_m, sml_const.out_s,
                                 sml_var.out_g, sml_var.out_u,,

                                 sml_const.out_f, med_const.out_f,
                                 lrg_const.out_f,,

                                 med_const.test, med_var.test,
                                 med_const.out_g, med_const.out_u,
                                 med_const.out_m, med_const.out_s,
                                 med_var.out_g, med_var.out_u,,

                                 lrg_const.test, lrg_var.test,
                                 lrg_const.out_g, lrg_const.out_u,
                                 lrg_const.out_m, lrg_const.out_s,
                                 lrg_var.out_g, lrg_var.out_u);
     #1.3 $finish(0);
  end
endmodule

/*
 * These should have a positive edge at 1234 time ticks.
 */
// Check that constant delays are scaled correctly.
module sml_const;
  reg test, in;
  wire out_g, out_u, out_m, out_s, out_f;
  not #(1.134, 0) dut_g (out_g, in);
  not_u #(1.134, 0) dut_u (out_u, in);
  sml_inv dut_m (out_m, in);
  gate_sdf dut_f (out_f, in);
  sml_sdf dut_s (out_s, in);
  initial begin
    $sdf_annotate("ivltests/real_delay.sdf");
    $sdf_annotate("ivltests/real_delay_sml.sdf");
    in = 1'b1;
    in <= #0.1 1'b0;
    test = 1'b0;
    #1.234 test = 1'b1;
  end
endmodule

// Check that the specify delays are scaled correctly.
module sml_inv(out, in);
  output out;
  input in;
  wire out, in;

  assign out = ~in;

  specify
    (in => out) = (1.134, 0);
  endspecify
endmodule

// Check that the SDF delays scale correctly.
module sml_sdf(out, in);
  output out;
  input in;
  wire out, in;

  assign out = ~in;

  specify
    (in => out) = (0.5, 0.5);
  endspecify
endmodule


// Check that variable delays are scaled correctly.
module sml_var;
  reg test, in;
  real dly, dly2, dly3;
  wire out_g, out_u;
  not #(dly2, dly3) dut_g (out_g, in);
  not_u #(dly2, dly3) dut_u (out_u, in);
  initial begin
    in = 1'b1;
    in <= #0.1 1'b0;
    dly = 1.234;
    dly2 = 1.134;
    dly3 = 0.0;
    test = 1'b0;
    #(dly) test = 1'b1;
  end
endmodule

`timescale 1ns/10ps

/*
 * These should have a positive edge at 1230 time ticks.
 */
// Check that constant delays are scaled correctly.
module med_const;
  reg test, in;
  wire out_g, out_u, out_m, out_s, out_f;
  not #(1.134, 0) dut_g (out_g, in);
  not_u #(1.134, 0) dut_u (out_u, in);
  med_inv dut_m (out_m, in);
  gate_sdf dut_f (out_f, in);
  med_sdf dut_s (out_s, in);
  initial begin
    $sdf_annotate("ivltests/real_delay.sdf");
    $sdf_annotate("ivltests/real_delay_med.sdf");
    in = 1'b1;
    in <= #0.1 1'b0;
    test = 1'b0;
    #1.234 test = 1'b1;
  end
endmodule

// Check that the specify delays are scaled correctly.
module med_inv(out, in);
  output out;
  input in;
  wire out, in;

  assign out = ~in;

  specify
    (in => out) = (1.134, 0);
  endspecify
endmodule

// Check that the SDF delays scale correctly.
module med_sdf(out, in);
  output out;
  input in;
  wire out, in;

  assign out = ~in;

  specify
    (in => out) = (0.5, 0.5);
  endspecify
endmodule

// Check that variable delays are scaled correctly.
module med_var;
  reg test, in;
  real dly, dly2, dly3;
  wire out_g, out_u;
  not #(dly2, dly3) dut_g (out_g, in);
  not_u #(dly2, dly3) dut_u (out_u, in);
  initial begin
    in = 1'b1;
    in <= #0.1 1'b0;
    dly = 1.234;
    dly2 = 1.134;
    dly3 = 0.0;
    test = 1'b0;
    #(dly) test = 1'b1;
  end
endmodule

`timescale 1ns/100ps

/*
 * These should have a positive edge at 1200 time ticks.
 */
// Check that constant delays are scaled correctly.
module lrg_const;
  reg test, in;
  wire out_g, out_u, out_m, out_s, out_f;
  not #(1.134, 0) gate (out_g, in);
  not_u #(1.134, 0) dut_u (out_u, in);
  lrg_inv dut_m (out_m, in);
  gate_sdf dut_f (out_f, in);
  lrg_sdf dut_s (out_s, in);
  initial begin
    $sdf_annotate("ivltests/real_delay.sdf");
    $sdf_annotate("ivltests/real_delay_lrg.sdf");
    in = 1'b1;
    in <= #0.1 1'b0;
    test = 1'b0;
    #1.234 test = 1'b1;
  end
endmodule

// Check that the specify delays are scaled correctly.
module lrg_inv(out, in);
  output out;
  input in;
  wire out, in;

  assign out = ~in;

  specify
    (in => out) = (1.134, 0);
  endspecify
endmodule

// Check that the SDF delays scale correctly.
module lrg_sdf(out, in);
  output out;
  input in;
  wire out, in;

  assign out = ~in;

  specify
    (in => out) = (0.5, 0.5);
  endspecify
endmodule

// Check that variable delays are scaled correctly.
module lrg_var;
  reg test, in;
  real dly, dly2, dly3;
  wire out_g, out_u;
  not #(dly2, dly3) dut_g (out_g, in);
  not_u #(dly2, dly3) dut_u (out_u, in);
  initial begin
    in = 1'b1;
    in <= #0.1 1'b0;
    dly = 1.234;
    dly2 = 1.134;
    dly3 = 0.0;
    test = 1'b0;
    #(dly) test = 1'b1;
  end
endmodule
