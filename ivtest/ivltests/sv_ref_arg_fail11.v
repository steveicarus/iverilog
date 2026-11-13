// "this" can not be passed to a (non-const) ref argument.
class C;
  function automatic void set(ref C h); h = null; endfunction
  function void f(); set(this); endfunction
endclass
module m; initial begin C c; c = new; c.f(); end endmodule
