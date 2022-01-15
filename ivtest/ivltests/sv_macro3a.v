`define PREFIX		my_prefix
`define SUFFIX		my_suffix

`define BACKTICK	"`"

`define name1		`PREFIX``_```SUFFIX
`define name2(p,s)	p``_``s

`define stringify(text)	`"text`"

module test();

initial begin
  $display(`BACKTICK);
  $display(`stringify(`name1));
  $display(`stringify(`name2(`PREFIX, `SUFFIX)));
end

endmodule
