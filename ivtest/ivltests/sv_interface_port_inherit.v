// Check that interface port headers are inherited across bare port names.

module M(input logic a, I p, q, (* keep = 1 *) r, output wire y, z);

  assign p.value = a;
  assign q.value = !a;
  assign r.value = p.value ^ q.value;
  assign y = r.value;
  assign z = q.value;

endmodule

interface I;
  logic value;
endinterface

module test;

  I i_p();
  I i_q();
  I i_r();
  wire y, z;

  M i_m(1'b1, i_p, i_q, i_r, y, z);

  initial begin
    #1;
    if ({i_p.value, i_q.value, i_r.value, y, z} === 5'b10110)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
