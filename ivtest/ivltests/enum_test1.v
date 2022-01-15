module test;

  parameter    PVALUE = 12;
  localparam   LVALUE = 88;

  enum byte unsigned { UVAL[256] } unsignedbyte_enum;
  enum byte          { SVAL[100] } signedbyte_enum;
  enum               { ADD = 10, SUB[5], JMP[6:8]} E1; // page 28 LRM
  enum               { REGISTER[2] = 1, REGISTER[2:4] = 10 } vr; // page 28 LRM
  enum               { P[5] = PVALUE, Q, S[3] = LVALUE} par_enum;
  enum reg [2:0]     { state[8] } bin_enum;
  enum integer       {IDLE, XX='bx, XY='b01, YY='b10, XZ = 32'h1x2z3xxz} next_state;

  int i;

  initial begin
    // 1. Default anonymous enum data type should be int
    // don't know yet how to quickly check this
    //
    // 1. Checking initialisations
    //
    // a. If the first name is not assigned it should be zero
    if (UVAL0 !== 8'h0 || SVAL0 !== 8'h0)
      begin
      $display ("FAILED - First un-assigned element of enum type was not zero");
      $finish;
    end
    // b. Checking initials E1 and vr
    if (ADD != 10 || REGISTER0 != 1)
      begin
      $display ("FAILED - First initialised elements of enums E1 and vr were not elaborated properly");
      $finish;
    end
    // A name without a value is automatically assigned and increment of the value of the
    // previous name (Section 4.10 LRM)
    // c. checking initial values for SUB (0-4) in E1
    if (SUB0 != 11 || SUB1 != 12 || SUB2 != 13 || SUB3 != 14 || SUB4 != 15)
      begin
      $display ("FAILED - Initialised elements SUB (0-4) in enum E1 were not elaborated properly");
      $finish;
    end
    // c. checking initial values for JMP (6-8) in E1
    if (JMP6 != 16 || JMP7 != 17 || JMP8 != 18)
      begin
      $display ("FAILED - Initialised elements (6-8) JMP in enum E1 were not elaborated properly");
      $finish;
    end
    // c. checking initial values in vr
    if (REGISTER1 != 2 || REGISTER2 != 10 || REGISTER3 != 11 || REGISTER4 != 12)
      begin
      $display ("FAILED - Initialised elements REGISTER (1-4) in enum vr were not elaborated properly");
      $finish;
    end
    // c. checking hand-picked values in unsignedbyte_enum
    if (UVAL23 != 23 || UVAL91 != 91 || UVAL138 != 138 || UVAL207 != 207)
      begin
      $display ("FAILED - Initialised some UVAL  in enum unsignedbyte_enum were not elaborated properly");
      $display ("UVAL23 = %0d, UVAL91 = %0d, UVAL138 = %0d, UVAL207 = %0d", UVAL23, UVAL91, UVAL138, UVAL207);
      $finish;
    end
    // c. checking hand-picked values in signedbyte_enum
    if (SVAL7 != 7 || SVAL19 != 19 || SVAL87 != 87)
      begin
      $display ("FAILED - Initialised some SVAL  in enum signedbyte_enum were not elaborated properly");
      $display ("SVAL7 = %0d, SVAL19 = %0d, SVAL87 = %0d", SVAL7, UVAL91, SVAL19, SVAL87);
      $finish;
    end
    // c. checking final values in unsignedbyte_enum and signedbyte_enum
    if (UVAL255 != 255 || SVAL99 != 99)
      begin
      $display ("FAILED - Initialised final values UVAL and SVAL did not elaborate properly");
      $display ("UVAL255 = %0d, SVAL99 = %0d", UVAL255, SVAL99);
      $finish;
    end
    // d. checking xz values in next_state
    if (XX !== 'bx || XZ !== 32'h1x2z3xxz)
      begin
      $display ("FAILED - Initialised x,z values in next_state did not elaborate properly");
      $finish;
    end
    // e. constants elaborated from parameter
    if (P0 != PVALUE+0 || P1 != PVALUE+1 || P2 != PVALUE+2 || P3 != PVALUE+3 || P4 != PVALUE + 4 || Q != PVALUE+5)
      begin
      $display ("FAILED - Initialised values P in par_enum were not elaborated properly");
      $finish;
    end
    // f. constants elaborated from localparam
    if (S0 != LVALUE+0 || S1 != LVALUE+1 || S2 != LVALUE+2)
      begin
      $display ("FAILED - Initialised values S in par_enum were not elaborated properly");
      $finish;
    end
    #1;
    // g. checking num method
    if (unsignedbyte_enum.num != 256 || signedbyte_enum.num != 100  ||
        E1.num != 9 || vr.num != 5 || par_enum.num != 9 )
      begin
      $display ("FAILED - The num method does not report as expected");
      $finish;
    end
    // h. checking first method
    if (unsignedbyte_enum.first != 0 || signedbyte_enum.first != 0  ||
        E1.first != 10 || vr.first != 1 || par_enum.first != PVALUE )
      begin
      $display ("FAILED - The first method does not report as expected");
      $finish;
    end
    // i. checking last method
    if (unsignedbyte_enum.last != 255 || signedbyte_enum.last != 99  ||
        E1.last != 18 || vr.last != 12 || par_enum.last != LVALUE+2 )
      begin
      $display ("FAILED - The last method does not report as expected");
      $finish;
    end
    // checking the next method on unsignedbyte_enum
    unsignedbyte_enum = unsignedbyte_enum.first;
    for (i=1; i<=255; i=i+1) begin
       unsignedbyte_enum = unsignedbyte_enum.next;
       if (unsignedbyte_enum != i) begin
          $display ("FAILED - The next method does not report as expected for unsignedbyte_enum");
          $finish;
       end
    end
    unsignedbyte_enum = unsignedbyte_enum.next;
    // checking wrap to the first element for signedbyte_enum
    if (unsignedbyte_enum != unsignedbyte_enum.first) begin
          $display ("FAILED - The next method did not wrap to the first element for unsignedbyte_enum");
          $finish;
    end
    // checking the next method on signedbyte_enum
    signedbyte_enum = signedbyte_enum.first;
    for (i=1; i<100; i=i+1) begin
       signedbyte_enum = signedbyte_enum.next;
       if (signedbyte_enum != i) begin
          $display ("FAILED - The next method does not report as expected for signedbyte_enum");
          $finish;
       end
    end
    signedbyte_enum = signedbyte_enum.next;
    // checking wrap to the first element for signedbyte_enum
    if (signedbyte_enum != signedbyte_enum.first) begin
          $display ("FAILED - The next method did not wrap to the first element for signedbyte_enum");
          $finish;
    end
    // checking the next method on E1
    E1 = E1.first;
    for (i=E1.first; i<= E1.last; i=i+1) begin
       if (E1 != i) begin
          $display ("FAILED - The next method does not report as expected for E1");
          $finish;
       end
       E1 = E1.next;
    end
    // checking wrap to the first element in E1
    if (E1 != E1.first) begin
          $display ("FAILED - The next method did not wrap to the first element for E1");
          $finish;
       end
    // checking the next method on vr, manual walk
    vr = vr.first;
    vr = vr.next;
    if (vr != 2) begin
          $display ("FAILED - The next method does not report as expected for vr in element REGISTER1");
          $finish;
    end
    vr = vr.next;
    if (vr != 10) begin
          $display ("FAILED - The next method does not report as expected for vr in element REGISTER2");
          $finish;
    end
    vr = vr.next;
    if (vr != 11) begin
          $display ("FAILED - The next method does not report as expected for vr in element REGISTER3");
          $finish;
    end
    vr = vr.next;
    if (vr != 12) begin
          $display ("FAILED - The next method does not report as expected for vr in element REGISTER4");
          $finish;
    end
    // checking wrap to the first element in vr
    vr = vr.next;
    if (vr != vr.first) begin
          $display ("FAILED - The next method did not wrap to the first element for vr");
          $finish;
    end
    // checking the next method for bin_enum
    bin_enum = bin_enum.first;
    for (i=bin_enum.first; i<= bin_enum.last; i = i+1) begin
      if (bin_enum != i) begin
          $display ("FAILED - The next method does not report as expected for bin_enum");
          $finish;
       end
       bin_enum = bin_enum.next;
    end
    // checking wrap to the first element in bin_enum
    if (bin_enum != bin_enum.first) begin
          $display ("FAILED - The next method did not wrap to the first element for bin_enum");
          $finish;
    end
    $display ("PASSED");
  end


endmodule
