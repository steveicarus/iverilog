// Check that an ANSI-style UDP output initializer requires a reg output.

primitive udp_ansi_initial_nonreg_fail (output o = 1'b0, input i);
  table
    0 : 0;
    1 : 1;
  endtable
endprimitive
