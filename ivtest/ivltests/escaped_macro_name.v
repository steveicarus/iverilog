`define simple      "simple name"
`define \escaped    "escaped name"
`define \`name      "backtick name"
`define \`          "backtick"
`define \quote (x)  `"`\`"x`\`"`"
`define \`\`"       "escaped quote"

module test();

initial begin
  $display(`simple);
  $display(`\simple );
  $display(`escaped);
  $display(`\escaped );
  $display(`\`name );
  $display(`\` );
  $display(`quote(text));
  $display(`\quote (text));
  $display(`\`\`" );
end

endmodule
