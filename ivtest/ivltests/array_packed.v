// Check that packed arrays of all sorts get elaborated without an error and
// that the resulting type has the right packed width.

package p;
  typedef logic [2:0] vector;
endpackage

module test;

typedef bit bit2;
typedef logic [1:0] vector;

bit2 [1:0] b;
vector [2:0] l;
p::vector [3:0] scoped_pa;

typedef enum logic [7:0] {
  A
} E;

typedef E [1:0] EP;
typedef EP [2:0] EPP;

E e;
EP ep1;
E [1:0] ep2;
EP [2:0] epp1;
EPP epp2;
EPP [3:0] eppp;

enum logic [7:0] {
  B
} [1:0] ep3;

typedef struct packed {
  longint x;
} S1;

typedef struct packed {
  time t;
  integer i;

  logic [1:0] x;
  bit [3:0] y;
  int z;
  shortint w;

  E e;
  EP ep;

  S1 s;
} S2;

localparam S_SIZE = 64 + 32 + 2 + 4 + 32 + 16 + 8 + 8*2 + 64;

typedef S2 [3:0] SP;
typedef SP [9:0] SPP;

S2 s;
SP sp1;
S2 [3:0] sp2;
SP [9:0] spp1;
SPP spp2;
SPP [1:0] sppp;

struct packed {
  S2 s;
} [3:0] sp3;

bit failed = 1'b0;

initial begin
  // Packed arrays of basic types
  failed |= $bits(b) !== 2;
  failed |= $bits(l) !== 2 * 3;
  failed |= $bits(scoped_pa) !== 3 * 4;

  // Packed arrays of enums
  failed |= $bits(e) !== 8;
  failed |= $bits(ep1) !== $bits(e) * 2;
  failed |= $bits(ep2) !== $bits(ep1);
  failed |= $bits(ep3) !== $bits(ep1);
  failed |= $bits(epp1) !== $bits(ep1) * 3;
  failed |= $bits(epp2) !== $bits(epp1);
  failed |= $bits(eppp) !== $bits(epp1) * 4;

  // Packed arrays of structs
  failed |= $bits(s) !== S_SIZE;
  failed |= $bits(sp1) !== $bits(s) * 4;
  failed |= $bits(sp2) !== $bits(sp1);
  failed |= $bits(sp3) !== $bits(sp1);
  failed |= $bits(spp1) !== $bits(sp1) * 10;
  failed |= $bits(spp1) !== $bits(spp2);
  failed |= $bits(sppp) !== $bits(spp1) * 2;

  if (failed)
    $display("FAILED");
  else
    $display("PASSED");
end

endmodule
