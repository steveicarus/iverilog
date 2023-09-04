// Check that all types of constant expression are supported for attributes

module test;

localparam [7:0] x = 1;

// Binary operators
(* attr = x + x *)      reg attr0;
(* attr = x - x *)      reg attr1;
(* attr = x * x *)      reg attr2;
(* attr = x / x *)      reg attr3;
(* attr = x % x *)      reg attr4;
(* attr = x == x *)     reg attr5;
(* attr = x != x *)     reg attr6;
(* attr = x === x *)    reg attr7;
(* attr = x !== x *)    reg attr8;
(* attr = x && x *)     reg attr9;
(* attr = x || x *)     reg attr10;
(* attr = x ** x *)     reg attr11;
(* attr = x < x *)      reg attr12;
(* attr = x <= x *)     reg attr13;
(* attr = x > x *)      reg attr14;
(* attr = x >= x *)     reg attr15;
(* attr = x & x *)      reg attr16;
(* attr = x | x *)      reg attr17;
(* attr = x ^ x *)      reg attr18;
(* attr = x ^~ x *)     reg attr19;
(* attr = x >> x *)     reg attr20;
(* attr = x << x *)     reg attr21;
(* attr = x >>> x *)    reg attr22;
(* attr = x <<< x *)    reg attr23;

// Unary operators
(* attr = +x *)         reg attr24;
(* attr = -x *)         reg attr25;
(* attr = !x *)         reg attr26;
(* attr = ~x *)         reg attr27;
(* attr = &x *)         reg attr28;
(* attr = ~&x *)        reg attr29;
(* attr = |x *)         reg attr30;
(* attr = ~|x *)        reg attr31;
(* attr = ^x *)         reg attr32;
(* attr = ~^x *)        reg attr33;

// Ternary operator
(* attr = x ? x : x *)  reg attr34;

// Concat
(* attr = {x,x} *)      reg attr35;
(* attr = {3{x}} *)     reg attr36;

// Part select
(* attr = x[0] *)       reg attr37;
(* attr = x[1:0] *)     reg attr38;
(* attr = x[0+:1] *)    reg attr39;
(* attr = x[1-:1] *)    reg attr40;

// Parenthesis
(* attr = (x) *)        reg attr41;

// Literals
(* attr = 10 *)         reg attr42;
(* attr = 32'h20 *)     reg attr43;
(* attr = "test" *)     reg attr44;

// System function
(* attr = $clog2(10) *) reg attr45;

// Function
function fn;
  input x;
  fn = x*2;
endfunction

(* attr = fn(10) *)     reg attr46;

// Macro escape only in a macro declaration
`define MACRO(arg) (* attr = `"`\`"arg`\`"`" *)
`MACRO(test) reg attr47;

initial begin
  $display("PASSED");
end

endmodule
