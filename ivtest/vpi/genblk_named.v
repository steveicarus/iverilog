module test;

parameter TRUE = 1;

genvar i;
genvar j;

for (i = 0; i < 2; i = i + 1) begin : l1
  reg r1 = 1;
end

for (i = 0; i < 2; i = i + 1) begin : l2
  for (j = 0; j < 2; j = j + 1) begin : l1
    reg r2 = 1;
  end
end

for (i = 0; i < 2; i = i + 1) begin : l3
  case (TRUE)
    0: begin : c1
         reg r3a = 1;
       end
    1: begin : c1
         reg r3b = 1;
       end
  endcase
end

for (i = 0; i < 2; i = i + 1) begin : l4
  if (TRUE)
    begin : i1
      reg r4a = 1;
    end
  else
    begin : i1
      reg r4b = 1;
    end
end

for (i = 0; i < 2; i = i + 1) begin : l5
  if (!TRUE)
    begin : i1
      reg r5a = 1;
    end
  else
    begin : i1
      if (TRUE)
        begin : i1
          reg r5b = 1;
        end
      else
        begin : i1
          reg r5c = 1;
        end
    end
end

for (i = 0; i < 2; i = i + 1) begin : l6
  if (!TRUE)
    begin : i1
      reg r6a = 1;
    end
  else
    begin : i1
      if (!TRUE)
        begin : i1
          reg r6b = 1;
        end
      else
        begin : i1
          reg r6c = 1;
        end
    end
end

case (TRUE)
  0: begin : c1
       reg r7a = 1;
     end
  1: begin : c1
       reg r7b = 1;
     end
endcase

case (TRUE)
  0: begin : c2
       case (TRUE)
         0: begin : c1
              reg r8a = 1;
            end
         1: begin : c1
              reg r8b = 1;
            end
       endcase
     end
  1: begin : c2
       case (TRUE)
         0: begin : c1
              reg r8c = 1;
            end
         1: begin : c1
              reg r8d = 1;
            end
       endcase
     end
endcase

case (TRUE)
  0: begin : c3
       if (TRUE)
         begin : i1
           reg r9a = 1;
         end
       else
         begin : i1
           reg r9b = 1;
         end
     end
  1: begin : c3
       if (TRUE)
         begin : i1
           reg r9c = 1;
         end
       else
         begin : i1
           reg r9d = 1;
         end
     end
endcase

case (TRUE)
  0: begin : c4
       if (!TRUE)
         begin : i1
           reg r10a = 1;
         end
       else
         begin : i1
           if (TRUE)
             begin : i1
               reg r10b = 1;
             end
           else
             begin : i1
               reg r10c = 1;
             end
         end
     end
  1: begin : c4
       if (!TRUE)
         begin : i1
           reg r10d = 1;
         end
       else
         begin : i1
           if (TRUE)
             begin : i1
               reg r10e = 1;
             end
           else
             begin : i1
               reg r10f = 1;
             end
         end
     end
endcase

case (TRUE)
  0: begin : c5
       if (!TRUE)
         begin : i1
           reg r11a = 1;
         end
       else
         begin : i1
           if (!TRUE)
             begin : i1
               reg r11b = 1;
             end
           else
             begin : i1
               reg r11c = 1;
             end
         end
     end
  1: begin : c5
       if (!TRUE)
         begin : i1
           reg r11d = 1;
         end
       else
         begin : i1
           if (!TRUE)
             begin : i1
               reg r11e = 1;
             end
           else
             begin : i1
               reg r11f = 1;
             end
         end
     end
endcase

case (TRUE)
  0: begin : c6
       if (!TRUE)
         begin : i1
           reg r12a = 1;
         end
       else if (TRUE)
         begin : i1
           reg r12b = 1;
         end
       else
         begin : i1
           reg r12c = 1;
         end
     end
  1: begin : c6
       if (!TRUE)
         begin : i1
           reg r12d = 1;
         end
       else if (TRUE)
         begin : i1
           reg r12e = 1;
         end
       else
         begin : i1
           reg r12f = 1;
         end
     end
endcase

case (TRUE)
  0: begin : c7
       if (!TRUE)
         begin : i1
           reg r13a = 1;
         end
       else if (!TRUE)
         begin : i1
           reg r13b = 1;
         end
       else
         begin : i1
           reg r13c = 1;
         end
     end
  1: begin : c7
       if (!TRUE)
         begin : i1
           reg r13d = 1;
         end
       else if (!TRUE)
         begin : i1
           reg r13e = 1;
         end
       else
         begin : i1
           reg r13f = 1;
         end
     end
endcase

if (TRUE)
  begin : i01
    reg r14a = 1;
  end
else
  begin : i01
    reg r14b = 1;
  end

if (!TRUE)
  begin : i02
    reg r15a = 1;
  end
else
  begin : i02
    if (TRUE)
      begin : i1
        reg r15b = 1;
      end
    else
      begin : i1
        reg r15c = 1;
      end
  end

if (!TRUE)
  begin : i03
    reg r16a = 1;
  end
else
  begin : i03
    if (!TRUE)
      begin : i1
        reg r16b = 1;
      end
    else
      begin : i1
        reg r16c = 1;
      end
  end

if (!TRUE)
  begin : i04
    reg r17a = 1;
  end
else if (TRUE)
  begin : i04
    reg r17b = 1;
  end
else
  begin : i04
    reg r17c = 1;
  end

if (!TRUE)
  begin : i05
    reg r18a = 1;
  end
else if (!TRUE)
  begin : i05
    reg r18b = 1;
  end
else
  begin : i05
    reg r18c = 1;
  end

if (TRUE)
  begin : i06
    if (TRUE)
      begin : i1
        reg r19a = 1;
      end
    else
      begin : i1
        reg r19b = 1;
      end
  end
else
  begin : i06
    reg r19c = 1;
  end

if (TRUE)
  begin : i07
    if (!TRUE)
      begin : i1
        reg r20a = 1;
      end
    else
      begin : i1
        reg r20b = 1;
      end
  end
else
  begin : i07
    reg r20c = 1;
  end

if (TRUE)
  begin : i08
    case (TRUE)
      0: begin : c1
           reg r21a = 1;
         end
      1: begin : c1
           reg r21b = 1;
         end
    endcase
  end
else
  begin : i08
    case (TRUE)
      0: begin : c1
           reg r21c = 1;
         end
      1: begin : c1
           reg r21d = 1;
         end
    endcase
  end

if (!TRUE)
  begin : i09
    case (TRUE)
      0: begin : c1
           reg r22a = 1;
         end
      1: begin : c1
           reg r22b = 1;
         end
    endcase
  end
else if (TRUE)
  begin : i09
    case (TRUE)
      0: begin : c1
           reg r22c = 1;
         end
      1: begin : c1
           reg r22d = 1;
         end
    endcase
  end
else
  begin : i09
    case (TRUE)
      0: begin : c1
           reg r22e = 1;
         end
      1: begin : c1
           reg r22f = 1;
         end
    endcase
  end

if (!TRUE)
  begin : i10
    case (TRUE)
      0: begin : c1
           reg r23a = 1;
         end
      1: begin : c1
           reg r23b = 1;
         end
    endcase
  end
else if (!TRUE)
  begin : i10
    case (TRUE)
      0: begin : c1
           reg r23c = 1;
         end
      1: begin : c1
           reg r23d = 1;
         end
    endcase
  end
else
  begin : i10
    case (TRUE)
      0: begin : c1
           reg r23e = 1;
         end
      1: begin : c1
           reg r23f = 1;
         end
    endcase
  end

initial begin
  $list_regs;
end

endmodule
