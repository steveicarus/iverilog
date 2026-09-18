// Check that an undefined package-qualified parameter type fails during
// elaboration.

package p;
endpackage

module test;

  parameter p::missing_type value = 0;

endmodule
