// Check that continued interface ports inherit the modport selector.

interface I;
  logic value;
  modport mp(input value);
endinterface

module M(I.mp p, q, r, output wire [2:0] y);

  assign y = {p.value, q.value, r.value};

endmodule

module test;

  I i_p();
  I i_q();
  I i_r();
  wire [2:0] y;

  assign i_p.value = 1'b1;
  assign i_q.value = 1'b0;
  assign i_r.value = 1'b1;

  M i_m(i_p, i_q, i_r, y);

  initial begin
    #1;
    if (y === 3'b101)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
