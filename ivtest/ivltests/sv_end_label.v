module top;
  initial begin: b_label
    $display("PASSED");
  end: b_label

  initial fork:fj_label
  join:fj_label

  initial fork:fja_label
  join_any:fja_label

  initial fork:fjn_label
  join_none:fjn_label

  task t_label;
  endtask: t_label

  task twa_label(input arg);
  endtask: twa_label

  function fn_label;
    input arg;
  endfunction: fn_label

  function fa_label(input in);
  endfunction: fa_label

endmodule:top

macromodule extra;
  parameter add_inv = 1;
  reg a;
  wire y, yb;
  pbuf dut(y, a);

  if (add_inv) begin: g_label
    pinv dut2(yb, y);
  end: g_label

endmodule: extra

package pkg;
endpackage: pkg

program pgm;
  class foo;
  endclass: foo
endprogram: pgm

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
endprimitive: pinv
