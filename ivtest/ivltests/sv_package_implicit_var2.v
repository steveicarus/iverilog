// Check that it is not possible to declare a variable in a package without an explicit data
// type for the variable.

pacakge P;
  [3:0] x; // This is a syntax error
endpackage
