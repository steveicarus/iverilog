`define PATH		/usr/local/bin/
`define STRINGIFY(x)	`"x`"

module test();

initial begin
  $display( `STRINGIFY(`PATH) );
end

endmodule
