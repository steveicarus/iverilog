module top;

  real ra = 1.0, rb = 1.0;
  wire real rdand, rdnand, rdor, rdnor, rdxor, rdxnor,
            rand, rnand, ror, rnor, rxor, rxnor,
            rls, rals, rrs, rars;
  wire [3:0] lsbr, alsbr, rsbr, arsbr;

  /* Test the reduction operators. */
  assign rdand = &ra;
  assign rdnand = ~&ra;
  assign rdor = |ra;
  assign rdnor = ~|ra;
  assign rdxor = ^ra;
  assign rdxnor = ~^ra;

  /* Test the bit-wise operators. */
  assign rand = ra & rb;
  assign rnand = ra ~& rb;
  assign ror = ra | rb;
  assign rnor = ra ~| rb;
  assign rxor = ra ^ rb;
  assign rxnor = ra ~^ rb;

  /* Test the shift operators. */
  assign rls = ra << rb;
  assign rals = ra <<< rb;
  assign rrs = ra >> rb;
  assign rars = ra >>> rb;

  assign lsbr = 4'b1010 << rb;
  assign alsbr = 4'b1010 <<< rb;
  assign rsbr = 4'b1010 >> rb;
  assign arsbr = 4'b1010 >>> rb;
endmodule
