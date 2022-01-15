package p1;

endpackage

module m1;

endmodule

package p2;

endpackage

module m2;

endmodule

module test;

initial begin
  $list_packages;
  $list_modules;
end

endmodule
