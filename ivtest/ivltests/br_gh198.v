module octal();

reg [5:0] var1;
reg [4:0] var2;

initial begin
  var1 = 6'o00; $displayo($signed(var1));
  var1 = 6'o01; $displayo($signed(var1));
  var1 = 6'o02; $displayo($signed(var1));
  var1 = 6'o03; $displayo($signed(var1));
  var1 = 6'o04; $displayo($signed(var1));
  var1 = 6'o05; $displayo($signed(var1));
  var1 = 6'o06; $displayo($signed(var1));
  var1 = 6'o07; $displayo($signed(var1));
  var1 = 6'o10; $displayo($signed(var1));
  var1 = 6'o20; $displayo($signed(var1));
  var1 = 6'o30; $displayo($signed(var1));
  var1 = 6'o40; $displayo($signed(var1));
  var1 = 6'o50; $displayo($signed(var1));
  var1 = 6'o60; $displayo($signed(var1));
  var1 = 6'o70; $displayo($signed(var1));
  var1 = 6'o17; $displayo($signed(var1));
  var1 = 6'o26; $displayo($signed(var1));
  var1 = 6'o35; $displayo($signed(var1));
  var1 = 6'o44; $displayo($signed(var1));
  var1 = 6'o53; $displayo($signed(var1));
  var1 = 6'o62; $displayo($signed(var1));
  var1 = 6'o71; $displayo($signed(var1));
  $display("");
  var2 = 6'o00; $displayo($signed(var2));
  var2 = 6'o01; $displayo($signed(var2));
  var2 = 6'o02; $displayo($signed(var2));
  var2 = 6'o03; $displayo($signed(var2));
  var2 = 6'o04; $displayo($signed(var2));
  var2 = 6'o05; $displayo($signed(var2));
  var2 = 6'o06; $displayo($signed(var2));
  var2 = 6'o07; $displayo($signed(var2));
  var2 = 6'o10; $displayo($signed(var2));
  var2 = 6'o20; $displayo($signed(var2));
  var2 = 6'o30; $displayo($signed(var2));
  var2 = 6'o17; $displayo($signed(var2));
  var2 = 6'o26; $displayo($signed(var2));
  var2 = 6'o35; $displayo($signed(var2));
end

endmodule
