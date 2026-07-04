// Check that an old-style UDP initial statement requires a reg output.

primitive udp_initial_nonreg_fail (o, i);
  output o;
  input i;
  initial o = 1'b0;

  table
    0 : 0;
    1 : 1;
  endtable
endprimitive
