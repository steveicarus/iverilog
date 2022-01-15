module p #(parameter a = 1, b = 2) ();

  typedef logic [b:a] vector_t;

  vector_t v;

  initial begin
    v = ~0;
    #b $display("%m %0d %0d %b", a, b, v);
  end
endmodule

module m;

p #(1,2) p1();
p #(1,4) p2();

endmodule
