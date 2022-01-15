module test;
  reg blah [63:0];
  initial blah = 0; // This should generate an error message.
endmodule
