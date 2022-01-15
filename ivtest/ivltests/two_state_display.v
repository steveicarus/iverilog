module top;
  bit [2:-1] vec = 4'b1001;
  bit btvar = 0;
  byte bvar = 0;
  shortint svar = 0;
  int ivar = 0;
  longint lvar = 0;
  initial begin
    if ((vec[-1] != 1) && (vec[0] != 0) &&
        (vec[1] != 0) && (vec[2] != 1)) begin
      $display("Failed to select vector bits correctly");
      $finish;
    end
    $display("Vec:   ", vec);
    $display("Bit:   ", btvar);
    $display("Byte:  ", bvar);
    $display("Short: ", svar);
    $display("Int:   ", ivar);
    $display("Long:  ", lvar);
    $display("Monitor results:");

    $monitor("Time: ", $stime,
           "\n  Bit:   ", btvar,
           "\n  Byte:  ", bvar,
           "\n  Short: ", svar,
           "\n  Int:   ", ivar,
           "\n  Long:  ", lvar);

    #1 btvar = 1;
    #1 bvar = 1;
    #1 svar = 1;
    #1 ivar = 1;
    #1 lvar = 1;
  end
endmodule
