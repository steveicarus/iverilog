// This just tests the compiler accepts the syntax. It needs to be improved
// when deferred assumeions are supported.
module test();

integer i = 1;

initial begin
  assume #0 (i == 1);
  assume #0 (i == 0);
  assume #0 (i == 1) else $display("Check 3 : this shouldn't be displayed");
  assume #0 (i == 0) else $display("Check 4 : this should be displayed");
  assume #0 (i == 1) $display("Check 5 : this should be displayed");
  assume #0 (i == 0) $display("Check 6 : this shouldn't be displayed");
  assume #0 (i == 1) $display("Check 7 : this should be displayed");
    else $display("Check 7 : this shouldn't be displayed");
  assume #0 (i == 0) $display("Check 8 : this shouldn't be displayed");
    else $display("Check 8 : this should be displayed");
end

endmodule
