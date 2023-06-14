// Check the line and file information for errors related to implicit named port
// connections are correct.

module M(
  output o
);
endmodule

module test;
  M i_m(.o); // Error, no net named o
endmodule
