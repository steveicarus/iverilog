module top;
  real var1, var2;
  reg [7:0] bvar;
  reg result;

  wire r_a = &var1;
  wire r_o = |var1;
  wire r_x = ^var1;
  wire r_na = ~&var1;
  wire r_no = ~|var1;
  wire r_xn1 = ~^var1;
  wire r_xn2 = ^~var1;

  wire r_b_a = var1 & var2;
  wire r_b_o = var1 | var2;
  wire r_b_x = var1 ^ var2;
  wire r_b_na = var1 ~& var2;
  wire r_b_no = var1 ~| var2;
  wire r_b_xn1 = var1 ~^ var2;
  wire r_b_xn2 = var1 ^~ var2;

  wire r_eeq = var1 === var2;
  wire r_neeq = var1 !== var2;

  wire r_ls = var1 << var2;
  wire r_als = var1 <<< var2;
  wire r_rs = var1 >> var2;
  wire r_ars = var1 >>> var2;

  wire r_con = {var1};
  wire r_rep = {2.0{var1}};

  initial begin
    var1 = 1.0;
    var2 = 2.0;

    #1;

    /* These should all fail in the compiler. */
    result = &var1;
    result = |var1;
    result = ^var1;
    result = ~&var1;
    result = ~|var1;
    result = ~^var1;
    result = ^~var1;

    result = var1 & var2;
    result = var1 | var2;
    result = var1 ^ var2;
    result = var1 ~& var2;
    result = var1 ~| var2;
    result = var1 ~^ var2;
    result = var1 ^~ var2;

    result = var1 === var2;
    result = var1 !== var2;

    bvar = var1 << var2;
    bvar = var1 <<< var2;
    bvar = var1 >> var2;
    bvar = var1 >>> var2;

    bvar = {var1};
    bvar = {2.0{var1}};

    $display("Failed");
  end
endmodule
