// PR1841300
// The output should be:
// a is '14'; b is 'fffffff4'; c is 'fffffff4'
module test;
   reg[4:0] a;
   reg [31:0] b, c;
   initial begin
      a = 5'b10100;
      b = $signed(a);
      c = $signed(_$Finv5(32'hab));
      $display("a is '%h'; b is '%h'; c is '%h'", a, b, c);
   end
   function [4:0] _$Finv5;
      input l;
      reg [4:0] l;
      _$Finv5 = ~l;
   endfunction
endmodule
