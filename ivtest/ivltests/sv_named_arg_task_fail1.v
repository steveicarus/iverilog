// Check that an error is reported when trying to bind an argument by nae that
// does not exist

module test;

  task t(integer a, integer b);
    $display("FAILED");
  endtask

  initial begin
    t(.b(2), .c(1));
  end

endmodule
