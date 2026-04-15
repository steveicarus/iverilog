module br_gh1323_lib #(parameter `typ(X) = 0) ();

initial begin
  if (X == 1)
    $display("PASSED");
  else
    $display("FAILED");
end

endmodule
