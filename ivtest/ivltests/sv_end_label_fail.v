module top;
  initial begin: b_label
    $display("FAILED");
  end: b_label_f

  initial fork:fj_label
  join:fj_label_f

  initial fork:fja_label
  join_any:fja_label_f

  initial fork:fjn_label
  join_none:fjn_label_f

  task t_label;
  endtask: t_label_f

  task twa_label(input arg);
  endtask: twa_label_f

  function fn_label;
    input arg;
  endfunction: fn_label_f

  function fa_label(input in);
  endfunction: fa_label_f

endmodule:top_f

macromodule extra;
  parameter add_inv = 1;
  reg a;
  wire y, yb;
  pbuf dut(y, a);

  if (add_inv) begin: g_label
    pinv dut2(yb, y);
  end: g_label_f

endmodule: extra_f

package pkg;
endpackage: pkg_f

program pgm;
  class foo;
  endclass: foo_f
endprogram: pgm_f

primitive pbuf (out, in);
  output out;
  input in;
  table
    0 : 0;
    1 : 1;
  endtable
endprimitive: pbuf

primitive pinv (output out, input in);
  table
    0 : 1;
    1 : 0;
  endtable
endprimitive: pinv_f
