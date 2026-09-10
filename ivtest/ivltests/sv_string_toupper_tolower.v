// Tests string case conversion functions

module sv_string_toupper_tolower();

string str;

initial begin
  str = "UPPERCASE lowercase";

  if (str.toupper() != "UPPERCASE LOWERCASE") begin
    $display("FAILED");
    $finish();
  end

  // Make sure string wasn't changed in-place by toupper()
  if (str != "UPPERCASE lowercase") begin
    $display("FAILED");
    $finish();
  end

  if (str.tolower() != "uppercase lowercase") begin
    $display("FAILED");
    $finish();
  end

  // Make sure string wasn't changed in-place by tolower()
  if (str != "UPPERCASE lowercase") begin
    $display("FAILED");
    $finish();
  end

  $display("PASSED");
end

endmodule

