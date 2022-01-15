/*
 * This test is based on PR#905
 */
module bug();

initial begin
  $displayh("", 99);
  $displayo("", 99);
  $displayb("", 99);
end

endmodule // bug
