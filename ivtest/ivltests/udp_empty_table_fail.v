// Check that an empty old-style UDP table generates an error.

primitive udp_empty_table_fail (o, i);
  output o;
  input i;

  table
  endtable
endprimitive
