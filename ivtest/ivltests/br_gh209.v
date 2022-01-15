module test();

integer f;

initial begin
  f = $fopen("work/br_gh209.dat");
  $fwrite(f, "%c%c%c%c", 8'h00, 8'h01, 8'h02, 8'h03);
end

endmodule
