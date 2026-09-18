// Check that an empty index on a hierarchy identifier is rejected when the
// hierarchy name is also visible as a type identifier.

package p;
  typedef integer SCOPE;
endpackage

import p::*;

module test;
  initial $display("%0d", SCOPE[].value);

  generate
    if (1) begin : SCOPE
      integer value;
    end
  endgenerate
endmodule
