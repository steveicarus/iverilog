// Check that continued interface ports use their own unpacked dimensions.

interface I;
  logic value;
endinterface

module M(I p[1:0], q, r[3:2], output wire [4:0] y);

  assign y = {p[1].value, p[0].value, q.value, r[3].value, r[2].value};

endmodule

module test;

  I i_p[1:0]();
  I i_q();
  I i_r[3:2]();
  wire [4:0] y;

  assign i_p[1].value = 1'b1;
  assign i_p[0].value = 1'b0;
  assign i_q.value = 1'b1;
  assign i_r[3].value = 1'b0;
  assign i_r[2].value = 1'b1;

  M i_m(i_p, i_q, i_r, y);

  initial begin
    #1;
    if (y === 5'b10101)
      $display("PASSED");
    else
      $display("FAILED");
  end

endmodule
