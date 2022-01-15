module top;

  // A join_any will always take the shortest path
  always fork
    #2;
    #1;
  join_none

  initial begin
     $display("FAILED");
     #1;
     $finish;
  end

endmodule
