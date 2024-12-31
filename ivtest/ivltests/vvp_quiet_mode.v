module test();

initial begin
  $display("This should be suppressed");
  $fdisplay(32'h00000001, "This should be suppressed");
  $fdisplay(32'h80000001, "This should be displayed");
  $finish(1);
end

endmodule
