// Check that an undefined package-qualified port type fails during elaboration.

package p;
endpackage

module test(
  input p::missing_type value // Error: p::missing_type is not a type.
);
endmodule
