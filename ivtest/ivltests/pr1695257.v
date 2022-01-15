module test(CL, Q_data, D);
  parameter
      Bits = 84;
  input CL;
  output [Bits-1 : 0] Q_data;
  input  [Bits-1 : 0] D;

  reg WENreg;
  reg ICGFlag;

  specify
    specparam

      taa = 1.0;

    if (WENreg && !ICGFlag) (CL *> (Q_data[0] : D[0])) = (taa, taa);
  endspecify

endmodule
