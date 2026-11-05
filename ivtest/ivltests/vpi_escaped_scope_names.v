// Check that VPI reports the original instance and module definition names
// when both contain double quotes. Escaping one name must not overwrite the other.

module \M"name ;
endmodule

module test;
  \M"name \i"m ();

  initial begin
    if ($check_scope_names(\i"m ) !== 1) $display("FAILED");
    else $display("PASSED");
  end
endmodule
