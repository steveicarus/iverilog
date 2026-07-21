// Accellera Standard V2.8.1 Open Verification Library (OVL).
// Accellera Copyright (c) 2005-2014. All rights reserved.

`ifdef OVL_SYNTHESIS
`else
  integer error_count;
  integer cover_count;
  initial error_count = 0;
  initial cover_count = 0;
`endif // OVL_SYNTHESIS


  task ovl_error_t;
    input             xcheck;
    input [8*128-1:0] err_msg;
    reg   [8*16-1:0]  err_typ;
  begin
  `ifdef OVL_SYNTHESIS
  `else
    case (severity_level)
      `OVL_FATAL   : err_typ = "OVL_FATAL";
      `OVL_ERROR   : err_typ = "OVL_ERROR";
      `OVL_WARNING : err_typ = "OVL_WARNING";
      `OVL_INFO    : err_typ = "OVL_INFO";
      default      :
        begin
          err_typ = "OVL_ERROR";
          $display("OVL_ERROR: Illegal option used in parameter severity_level, setting message type to OVL_ERROR : time %0t : %m", $time);
        end
    endcase

    `ifdef OVL_MAX_REPORT_ERROR
      if (error_count < `OVL_MAX_REPORT_ERROR)
    `endif
        case (property_type)
          `OVL_ASSERT,
          `OVL_ASSUME         : begin
            $display("%s : (%0d) \n%0t : %0s : %0s : %0s : %0s : severity %0d : %m", 
			    `__FILE__,
			    `__LINE__,
			    $time, 
			    err_typ, assert_name, msg, err_msg, severity_level);
            error_count = error_count + 1;
          end
          `OVL_ASSERT_2STATE,
          `OVL_ASSUME_2STATE  : begin
            if (xcheck == `OVL_FIRE_2STATE) begin
              $display("%s : %s : %s : %0s : severity %0d : time %0t : %m", err_typ, assert_name, msg, err_msg, severity_level, $time);
            error_count = error_count + 1;
            end
          end
          `OVL_IGNORE         : begin end
          default             : begin
            $display("%s : Illegal option used in parameter property_type, setting to OVL_ASSERT : time %0t : %m", err_typ, $time);
            $display("%s : (%0d) \n%0t : %s : %s : %s : %0s : severity %0d : %m", 
			    `__FILE__,
			    `__LINE__,
			    $time, 
			    err_typ, assert_name, msg, err_msg, severity_level);
          end
        endcase

    `ifdef OVL_FINISH_OFF
    `else
      if (severity_level == `OVL_FATAL) begin
        case (property_type)
          `OVL_ASSERT,
          `OVL_ASSUME         : begin
            ovl_finish_t;
          end
          `OVL_ASSERT_2STATE,
          `OVL_ASSUME_2STATE  : begin
              if (xcheck == `OVL_FIRE_2STATE) begin
                ovl_finish_t;
              end
            end
          `OVL_IGNORE         : begin end
          default             : begin end
        endcase
      end
    `endif // OVL_FINISH_OFF
  `endif // OVL_SYNTHESIS
  end
  endtask // ovl_error_t


  task ovl_finish_t;
  begin
    `ifdef OVL_SYNTHESIS
    `else
      #`OVL_RUNTIME_AFTER_FATAL $finish;
    `endif // OVL_SYNTHESIS
  end
  endtask // ovl_finish_t


  task ovl_init_msg_t;
  begin
    `ifdef OVL_SYNTHESIS
    `else
      case (property_type)
        `OVL_ASSERT,
        `OVL_ASSUME,
        `OVL_ASSERT_2STATE,
        `OVL_ASSUME_2STATE : begin
          `ifdef OVL_SYNTHESIS
          `else
            `ifdef OVL_INIT_COUNT
              #0.1 `OVL_INIT_COUNT = `OVL_INIT_COUNT + 1;
            `else
              $display("OVL_NOTE: %s: %s initialized @ %m Severity: %0d, Message: %s", `OVL_VERSION, assert_name,severity_level, msg);
            `endif
          `endif // OVL_SYNTHESIS
         end
         `OVL_IGNORE : begin
            // do nothing
         end
       default : $display("OVL_ERROR: Illegal option used in parameter property_type : %m");
      endcase
    `endif // OVL_SYNTHESIS
  end
  endtask // ovl_init_msg_t


  task ovl_cover_t;
    input [8*128-1:0] cvr_msg;
  begin
   `ifdef OVL_SYNTHESIS
   `else
     cover_count = cover_count + 1;
     `ifdef OVL_MAX_REPORT_COVER_POINT
       if (cover_count <= `OVL_MAX_REPORT_COVER_POINT) begin
     `endif
        if (coverage_level > `OVL_COVER_ALL)
          $display("OVL_ERROR: Illegal option used in parameter coverage_level : time %0t : %m", $time);
        else
          $display("OVL_COVER_POINT : %s : %0s : time %0t : %m", assert_name, cvr_msg, $time);
     `ifdef OVL_MAX_REPORT_COVER_POINT
       end
     `endif
   `endif // OVL_SYNTHESIS
  end
  endtask // ovl_cover_t


`ifdef OVL_SVA
  // FUNCTION THAT CALCULATES THE LOG BASE 2 OF A NUMBER
  // ========
  // NOTE: only used in sva05
  function integer log2;
    input integer x;
    integer i;
    integer result;
  begin
    result = 1;
    if (x <= 0) result = -1;
    else
      for (i = 0; (1<<i) <= x; i=i+1) result = i+1;
    log2 = result;
  end
  endfunction
`endif // OVL_SVA


  function ovl_fire_2state_f;
    input   property_type;
    integer property_type;
  begin
    case (property_type)
      `OVL_ASSERT,
      `OVL_ASSUME        : ovl_fire_2state_f = 1'b1;
      `OVL_ASSERT_2STATE,
      `OVL_ASSUME_2STATE : ovl_fire_2state_f = 1'b1;
      `OVL_IGNORE        : ovl_fire_2state_f = 1'b0;
      default            : ovl_fire_2state_f = 1'b0;
    endcase
  end
  endfunction // ovl_fire_2state_f


  function ovl_fire_xcheck_f;
    input   property_type;
    integer property_type;
  begin
  `ifdef OVL_SYNTHESIS
    // fire_xcheck is not synthesizable
    ovl_fire_xcheck_f = 1'b0;
  `else
    case (property_type)
      `OVL_ASSERT,
      `OVL_ASSUME        : ovl_fire_xcheck_f = 1'b1;
      `OVL_ASSERT_2STATE,
      `OVL_ASSUME_2STATE : ovl_fire_xcheck_f = 1'b0;
      `OVL_IGNORE        : ovl_fire_xcheck_f = 1'b0;
      default            : ovl_fire_xcheck_f = 1'b0;
    endcase
  `endif // OVL_SYNTHESIS
  end
  endfunction // ovl_fire_xcheck_f

  function ovl_fire_cover_f;
    input   coverage_level;
    integer coverage_level;
  begin
    ovl_fire_cover_f = 1'b0;
    case (coverage_level)
      `OVL_COVER_SANITY,
      `OVL_COVER_BASIC,
      `OVL_COVER_CORNER,
      `OVL_COVER_STATISTIC,
      `OVL_COVER_ALL     : ovl_fire_cover_f = 1'b1;
      `OVL_COVER_NONE    : ovl_fire_cover_f = 1'b0;
      default            : ovl_fire_cover_f = 1'b0;
    endcase
  end
  endfunction // ovl_fire_cover_f

