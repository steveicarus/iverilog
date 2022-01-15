module test();

function void do_nothing();
  ;
endfunction

initial begin
  do_nothing();
  $display("PASSED");
end

endmodule
