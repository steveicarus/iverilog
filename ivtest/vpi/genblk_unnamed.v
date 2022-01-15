module test;

// force leading zero on outer scope genblk numbers
localparam genblk1 = 0;
localparam genblk2 = 0;
localparam genblk3 = 0;
localparam genblk4 = 0;
localparam genblk5 = 0;
localparam genblk6 = 0;
localparam genblk7 = 0;
localparam genblk8 = 0;
localparam genblk9 = 0;

parameter TRUE = 1;

genvar i;
genvar j;

for (i = 0; i < 2; i = i + 1) begin
  reg r1 = 1;
end

for (i = 0; i < 2; i = i + 1) begin
  for (j = 0; j < 2; j = j + 1) begin
    reg r2 = 1;
  end
end

for (i = 0; i < 2; i = i + 1) begin
  case (TRUE)
    0: begin
         reg r3a = 1;
       end
    1: begin
         reg r3b = 1;
       end
  endcase
end

for (i = 0; i < 2; i = i + 1) begin
  if (TRUE)
    begin
      reg r4a = 1;
    end
  else
    begin
      reg r4b = 1;
    end
end

for (i = 0; i < 2; i = i + 1) begin
  if (!TRUE)
    begin
      reg r5a = 1;
    end
  else
    begin
      if (TRUE)
        begin
          reg r5b = 1;
        end
      else
        begin
          reg r5c = 1;
        end
    end
end

for (i = 0; i < 2; i = i + 1) begin
  if (!TRUE)
    begin
      reg r6a = 1;
    end
  else
    begin
      if (!TRUE)
        begin
          reg r6b = 1;
        end
      else
        begin
          reg r6c = 1;
        end
    end
end

case (TRUE)
  0: begin
       reg r7a = 1;
     end
  1: begin
       reg r7b = 1;
     end
endcase

case (TRUE)
  0: begin
       case (TRUE)
         0: begin
              reg r8a = 1;
            end
         1: begin
              reg r8b = 1;
            end
       endcase
     end
  1: begin
       case (TRUE)
         0: begin
              reg r8c = 1;
            end
         1: begin
              reg r8d = 1;
            end
       endcase
     end
endcase

case (TRUE)
  0: begin
       if (TRUE)
         begin
           reg r9a = 1;
         end
       else
         begin
           reg r9b = 1;
         end
     end
  1: begin
       if (TRUE)
         begin
           reg r9c = 1;
         end
       else
         begin
           reg r9d = 1;
         end
     end
endcase

case (TRUE)
  0: begin
       if (!TRUE)
         begin
           reg r10a = 1;
         end
       else
         begin
           if (TRUE)
             begin
               reg r10b = 1;
             end
           else
             begin
               reg r10c = 1;
             end
         end
     end
  1: begin
       if (!TRUE)
         begin
           reg r10d = 1;
         end
       else
         begin
           if (TRUE)
             begin
               reg r10e = 1;
             end
           else
             begin
               reg r10f = 1;
             end
         end
     end
endcase

case (TRUE)
  0: begin
       if (!TRUE)
         begin
           reg r11a = 1;
         end
       else
         begin
           if (!TRUE)
             begin
               reg r11b = 1;
             end
           else
             begin
               reg r11c = 1;
             end
         end
     end
  1: begin
       if (!TRUE)
         begin
           reg r11d = 1;
         end
       else
         begin
           if (!TRUE)
             begin
               reg r11e = 1;
             end
           else
             begin
               reg r11f = 1;
             end
         end
     end
endcase

case (TRUE)
  0: begin
       if (!TRUE)
         begin
           reg r12a = 1;
         end
       else if (TRUE)
         begin
           reg r12b = 1;
         end
       else
         begin
           reg r12c = 1;
         end
     end
  1: begin
       if (!TRUE)
         begin
           reg r12d = 1;
         end
       else if (TRUE)
         begin
           reg r12e = 1;
         end
       else
         begin
           reg r12f = 1;
         end
     end
endcase

case (TRUE)
  0: begin
       if (!TRUE)
         begin
           reg r13a = 1;
         end
       else if (!TRUE)
         begin
           reg r13b = 1;
         end
       else
         begin
           reg r13c = 1;
         end
     end
  1: begin
       if (!TRUE)
         begin
           reg r13d = 1;
         end
       else if (!TRUE)
         begin
           reg r13e = 1;
         end
       else
         begin
           reg r13f = 1;
         end
     end
endcase

if (TRUE)
  begin
    reg r14a = 1;
  end
else
  begin
    reg r14b = 1;
  end

if (!TRUE)
  begin
    reg r15a = 1;
  end
else
  begin
    if (TRUE)
      begin
        reg r15b = 1;
      end
    else
      begin
        reg r15c = 1;
      end
  end

if (!TRUE)
  begin
    reg r16a = 1;
  end
else
  begin
    if (!TRUE)
      begin
        reg r16b = 1;
      end
    else
      begin
        reg r16c = 1;
      end
  end

if (!TRUE)
  begin
    reg r17a = 1;
  end
else if (TRUE)
  begin
    reg r17b = 1;
  end
else
  begin
    reg r17c = 1;
  end

if (!TRUE)
  begin
    reg r18a = 1;
  end
else if (!TRUE)
  begin
    reg r18b = 1;
  end
else
  begin
    reg r18c = 1;
  end

if (TRUE)
  begin
    if (TRUE)
      begin
        reg r19a = 1;
      end
    else
      begin
        reg r19b = 1;
      end
  end
else
  begin
    reg r19c = 1;
  end

if (TRUE)
  begin
    if (!TRUE)
      begin
        reg r20a = 1;
      end
    else
      begin
        reg r20b = 1;
      end
  end
else
  begin
    reg r20c = 1;
  end

if (TRUE)
  begin
    case (TRUE)
      0: begin
           reg r21a = 1;
         end
      1: begin
           reg r21b = 1;
         end
    endcase
  end
else
  begin
    case (TRUE)
      0: begin
           reg r21c = 1;
         end
      1: begin
           reg r21d = 1;
         end
    endcase
  end

if (!TRUE)
  begin
    case (TRUE)
      0: begin
           reg r22a = 1;
         end
      1: begin
           reg r22b = 1;
         end
    endcase
  end
else if (TRUE)
  begin
    case (TRUE)
      0: begin
           reg r22c = 1;
         end
      1: begin
           reg r22d = 1;
         end
    endcase
  end
else
  begin
    case (TRUE)
      0: begin
           reg r22e = 1;
         end
      1: begin
           reg r22f = 1;
         end
    endcase
  end

if (!TRUE)
  begin
    case (TRUE)
      0: begin
           reg r23a = 1;
         end
      1: begin
           reg r23b = 1;
         end
    endcase
  end
else if (!TRUE)
  begin
    case (TRUE)
      0: begin
           reg r23c = 1;
         end
      1: begin
           reg r23d = 1;
         end
    endcase
  end
else
  begin
    case (TRUE)
      0: begin
           reg r23e = 1;
         end
      1: begin
           reg r23f = 1;
         end
    endcase
  end

initial begin
  $list_regs;
end

endmodule
