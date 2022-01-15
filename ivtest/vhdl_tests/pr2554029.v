module test();
  a a (.pi(pi));
endmodule // test

module a(input pi);
  assign Pi = pi;
  b b(.Pi(Pi));
endmodule // a

module b(input Pi);
endmodule
