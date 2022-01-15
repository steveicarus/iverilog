module top;

  // A join_any will always take the shortest path
  always fork
    #0;
    #1;
  join_any

  initial begin
     $display("FAILED");
     #1;
     $finish;
  end

endmodule
