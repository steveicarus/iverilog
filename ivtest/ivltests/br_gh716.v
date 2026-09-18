module test();
  logic [7:0] bf [];

  task t1(output logic [7:0] buffer);
    buffer=0;
  endtask

  initial t1(bf);

endmodule
