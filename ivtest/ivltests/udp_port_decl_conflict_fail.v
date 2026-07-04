// Check that conflicting UDP port declarations generate an error.

primitive udp_port_decl_conflict_fail (o, i);
  output o;
  input o;
  input i;

  table
    0 : 0;
    1 : 1;
  endtable
endprimitive
