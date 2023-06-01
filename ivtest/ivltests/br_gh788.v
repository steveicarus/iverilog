module test();

task print_hex;

  input n;

  reg [7:0] n;

  begin
    $display("%h", n);
  end
endtask

initial begin
  print_hex(66);
end

endmodule
