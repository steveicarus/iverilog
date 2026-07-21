// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

  integer cnt;

`ifdef OVL_SYNTHESIS
`else
  initial begin
    cnt=0;
  end
`endif


`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  wire valid_push;
  wire valid_pop;

  assign valid_push = ~((^push) ^ (^push));
  assign valid_pop = ~((^pop) ^ (^pop));
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

`ifdef OVL_SHARED_CODE

  always @(posedge clk) begin
      if (`OVL_RESET_SIGNAL != 1'b0) begin // active low reset
        if ({push!=0,pop!=0} == 2'b10) begin // push
 `ifdef OVL_COVER_ON
            if (coverage_level != `OVL_COVER_NONE) begin
             if (OVL_COVER_BASIC_ON) begin //basic coverage
              ovl_cover_t("fifo_push covered");
             end
            end
 `endif // OVL_COVER_ON

          if ((cnt + push) > depth) begin
 `ifdef OVL_ASSERT_ON
             ovl_error_t(`OVL_FIRE_2STATE,"Fifo overflow detected");
 `endif // OVL_ASSERT_ON
          end
          else begin
            cnt <= cnt + push;
 `ifdef OVL_COVER_ON
              if (coverage_level != `OVL_COVER_NONE) begin
               if (OVL_COVER_CORNER_ON) begin //corner coverage
                if ((cnt + push) == depth) begin
                  ovl_cover_t("fifo_full covered");
                end
               end //corner coverage
              end
 `endif // OVL_COVER_ON
          end
        end
        else if ({push!=0,pop!=0} == 2'b01) begin // pop
 `ifdef OVL_COVER_ON
            if (coverage_level != `OVL_COVER_NONE) begin
             if (OVL_COVER_BASIC_ON) begin //basic coverage
              ovl_cover_t("fifo_pop covered");
             end
            end
 `endif // OVL_COVER_ON

          if (cnt < pop) begin
 `ifdef OVL_ASSERT_ON
             ovl_error_t(`OVL_FIRE_2STATE,"Fifo underflow detected");
 `endif // OVL_ASSERT_ON
          end
          else begin
            cnt <= cnt - pop;
 `ifdef OVL_COVER_ON
              if (coverage_level != `OVL_COVER_NONE) begin
               if (OVL_COVER_CORNER_ON) begin //corner coverage
                if ((cnt - pop) == 0) begin
                  ovl_cover_t("fifo_empty covered");
                end
               end //corner coverage
              end
 `endif // OVL_COVER_ON
          end
        end
        else if ({push!=0,pop!=0} == 2'b11) begin // push & pop
 `ifdef OVL_COVER_ON
            if (coverage_level != `OVL_COVER_NONE) begin
             if (OVL_COVER_CORNER_ON) begin //corner coverage
              ovl_cover_t("fifo_simultaneous_push_pop covered");
             end
            end

 `endif// OVL_COVER_ON

          if (!simultaneous_push_pop) begin
 `ifdef OVL_ASSERT_ON
             ovl_error_t(`OVL_FIRE_2STATE,"Illegal simultaneous push pop detected");
 `endif
          end
          else begin
            if ((cnt + push - pop) > depth) begin
 `ifdef OVL_ASSERT_ON
              ovl_error_t(`OVL_FIRE_2STATE,"Fifo overflow detected due to simultaneous push pop operations");
 `endif
            end
            else if ((cnt + push) < pop) begin
 `ifdef OVL_ASSERT_ON
              ovl_error_t(`OVL_FIRE_2STATE,"Fifo underflow detected due to simultaneous push pop operations");
 `endif
            end
            else begin
              cnt <= cnt + push - pop;
 `ifdef OVL_COVER_ON
                if (coverage_level != `OVL_COVER_NONE) begin
                 if (OVL_COVER_CORNER_ON) begin //corner coverage
                  if ((cnt + push - pop) == depth) begin
                    ovl_cover_t("fifo_full covered");
                  end
                  else if ((cnt + push - pop) == 0) begin
                    ovl_cover_t("fifo_empty covered");
                  end
                 end //corner coverage
                end
 `endif // OVL_COVER_ON
            end
          end
        end
      end
      else begin
        cnt <= 0;
      end
  end

`ifdef OVL_XCHECK_OFF
   //Do nothing
`else
  `ifdef OVL_IMPLICIT_XCHECK_OFF
    //Do nothing
  `else
  `ifdef OVL_ASSERT_ON

  always @ (posedge clk)
    begin
      if (`OVL_RESET_SIGNAL != 1'b0)
        begin
          if (valid_push == 1'b1)
            begin
              // Do nothing
            end
          else
            ovl_error_t(`OVL_FIRE_XCHECK,"push contains X or Z");
        end
    end

  always @ (posedge clk)
    begin
      if (`OVL_RESET_SIGNAL != 1'b0)
        begin
          if (valid_pop == 1'b1)
            begin
              // Do nothing
            end
          else
            ovl_error_t(`OVL_FIRE_XCHECK,"pop contains X or Z");
        end
    end

  `endif // OVL_ASSERT_ON
 `endif // OVL_IMPLICIT_XCHECK_OFF
`endif // OVL_XCHECK_OFF

`endif // OVL_SHARED_CODE
