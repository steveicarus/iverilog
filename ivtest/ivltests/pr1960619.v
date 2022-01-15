module test;
   initial begin: A
      reg [4:0]  a;
      reg [31:0] b, c, d;
      a = 5'h14;
      b = $signed(a);
      c =        {_$Finv5(_$Fsub8(_$Fadd8((32'haa ^ (32'hcc & _$Fsll32(_$Fsrl32(_$Fsll32(32'h78, 32'h2), 32'h3), 32'h1))), 32'h69), (32'h50 * 32'h2)))};
      d = $signed(_$Finv5(_$Fsub8(_$Fadd8((32'haa ^ (32'hcc & _$Fsll32(_$Fsrl32(_$Fsll32(32'h78, 32'h2), 32'h3), 32'h1))), 32'h69), (32'h50 * 32'h2))));
      $write("a is %0h; b is %0h; c is %0h; d is %0h\n", a, b, c, d);
   end

   function [4:0] _$Finv5;
      input	 l;
      reg [4:0]  l;
      _$Finv5 = ~l;
   endfunction

   function [7:0] _$Fsub8;
      input	 l,r;
      reg [7:0]  l,r;
      _$Fsub8 = l-r;
   endfunction

   function [7:0] _$Fadd8;
      input	 l,r;
      reg [7:0]  l,r;
      _$Fadd8 = l+r;
   endfunction

   function [31:0] _$Fsll32;
      input	 l,r;
      reg [31:0] l;
      integer    r;
      begin
	 if(r < 0) begin
	    if(r + 32 <= 0) _$Fsll32 = 32'b0;
	    else _$Fsll32 = l >> -r;
	 end else if(r > 0) begin
	    if(r >= 32) _$Fsll32 = 32'b0;
	    else _$Fsll32 = l << r;
	 end else _$Fsll32 = l;
      end
   endfunction

   function [31:0] _$Fsrl32;
      input l,r;
      reg [31:0] l;
      integer	 r;
      begin
	 if(r < 0) begin
	    if(r + 32 <= 0) _$Fsrl32 = 32'b0;
	    else _$Fsrl32 = l << -r;
	 end else if(r > 0) begin
	    if(r >= 32) _$Fsrl32 = 32'b0;
	    else _$Fsrl32 = l >> r;
	 end else _$Fsrl32 = l;
      end
   endfunction
endmodule
