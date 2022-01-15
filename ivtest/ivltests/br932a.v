module br932a();

task check_str(input string str);

begin
  $display("%s", str);
  if (str == "br932a")
    $display("PASSED");
  else
    $display("FAILED");
end

endtask

initial begin
  check_str("br932a");
end

endmodule
