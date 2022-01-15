// Check non-constant port default value is rejected
// in constant context.

module test();

integer a;
integer b;

function integer k(integer i, integer j = a+b);
  k = i + j;
endfunction

localparam integer result = k(3);

endmodule
