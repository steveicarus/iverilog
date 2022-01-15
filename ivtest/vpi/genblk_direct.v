module test;

// test name collisions
parameter   genblk1 = 0;
localparam  genblk2 = 0;

typedef reg genblk3;

reg         genblk4;
wire        genblk5;
event       genblk6;

class genblk7;
endclass

function genblk8();
endfunction;

task genblk9;
endtask

parameter TRUE = 1;

genvar i;
genvar j;

for (i = 0; i < 2; i = i + 1)
  reg r1 = 1;

for (i = 0; i < 2; i = i + 1)
  for (j = 0; j < 2; j = j + 1)
    reg r2 = 1;

for (i = 0; i < 2; i = i + 1)
  case (TRUE)
    0: reg r3a = 1;
    1: reg r3b = 1;
  endcase

for (i = 0; i < 2; i = i + 1)
  if (TRUE)
    reg r4a = 1;
  else
    reg r4b = 1;

for (i = 0; i < 2; i = i + 1)
  if (!TRUE)
    reg r5a = 1;
  else if (TRUE)
    reg r5b = 1;
  else
    reg r5c = 1;

for (i = 0; i < 2; i = i + 1)
  if (!TRUE)
    reg r6a = 1;
  else if (!TRUE)
    reg r6b = 1;
  else
    reg r6c = 1;

case (TRUE)
  0: reg r7a = 1;
  1: reg r7b = 1;
endcase

case (TRUE)
  0: case (TRUE)
       0: reg r8a = 1;
       1: reg r8b = 1;
     endcase
  1: case (TRUE)
       0: reg r8c = 1;
       1: reg r8d = 1;
     endcase
endcase

case (TRUE)
  0: if (TRUE)
       reg r9a = 1;
     else
       reg r9b = 1;
  1: if (TRUE)
       reg r9c = 1;
     else
       reg r9d = 1;
endcase

case (TRUE)
  0: if (!TRUE)
       reg r10a = 1;
     else if (TRUE)
       reg r10b = 1;
     else
       reg r10c = 1;
  1: if (!TRUE)
       reg r10d = 1;
     else if (TRUE)
       reg r10e = 1;
     else
       reg r10f = 1;
endcase

case (TRUE)
  0: if (!TRUE)
       reg r11a = 1;
     else if (!TRUE)
       reg r11b = 1;
     else
       reg r11c = 1;
  1: if (!TRUE)
       reg r11d = 1;
     else if (!TRUE)
       reg r11e = 1;
     else
       reg r11f = 1;
endcase

if (TRUE)
  reg r12a = 1;
else
  reg r12b = 1;

if (!TRUE)
  reg r13a = 1;
else if (TRUE)
  reg r13b = 1;
else
  reg r13c = 1;

if (!TRUE)
  reg r14a = 1;
else if (!TRUE)
  reg r14b = 1;
else
  reg r14c = 1;

if (TRUE)
  if (TRUE)
    reg r15a = 1;
  else
    reg r15b = 1;
else
  reg r15c = 1;

if (TRUE)
  if (!TRUE)
    reg r16a = 1;
  else
    reg r16b = 1;
else
  reg r16c = 1;

if (TRUE)
  case (TRUE)
    0: reg r17a = 1;
    1: reg r17b = 1;
  endcase
else
  case (TRUE)
    0: reg r17c = 1;
    1: reg r17d = 1;
  endcase

if (!TRUE)
  case (TRUE)
    0: reg r18a = 1;
    1: reg r18b = 1;
  endcase
else if (TRUE)
  case (TRUE)
    0: reg r18c = 1;
    1: reg r18d = 1;
  endcase
else
  case (TRUE)
    0: reg r18e = 1;
    1: reg r18f = 1;
  endcase

if (!TRUE)
  case (TRUE)
    0: reg r19a = 1;
    1: reg r19b = 1;
  endcase
else if (!TRUE)
  case (TRUE)
    0: reg r19c = 1;
    1: reg r19d = 1;
  endcase
else
  case (TRUE)
    0: reg r19e = 1;
    1: reg r19f = 1;
  endcase

initial begin
  $list_regs;
end

endmodule
