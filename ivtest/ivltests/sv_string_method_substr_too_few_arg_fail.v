// Check that string substr() rejects too few arguments.

module test;
  string s;
  string t;

  initial begin
    t = s.substr(0);
  end
endmodule
