/*
 * Copyright (c) 2008 True Circuits Inc.
 *
 * Author: Kevin Cameron
 *
 *    This source code is free software; you can redistribute it
 *    and/or modify it in source code form under the terms of the GNU
 *    General Public License as published by the Free Software
 *    Foundation; either version 2 of the License, or (at your option)
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
 */

# include  <stdio.h>
# include  <stdlib.h>
# include  <string.h>
# include  <unistd.h>
# include  <dlfcn.h>
# include  <math.h>

# include  "vpi_user.h"

# include  "vpi_priv.h"
# include  "vpi_user.h"
# include  "acc_user.h"
# define DECL__IVL_PARM_NM
# include  "extpwl.h"

static int bs_debug;

static PLI_INT32 bindsigs_compiletf(PLI_BYTE8 *user_data)
{
  static int shown = 0;

  if (!bs_debug) {
    char *env = getenv("BS_DEBUG");
    if (env && !(bs_debug = atoi(env))) {
      bs_debug = 1;
    } else {
      bs_debug = -1;
    }
  }

  if (bs_debug > 0 && !shown++) {
    fprintf(stderr,"Bindsigs compiled in.\n");
  }

  return 0;
}

static SpcIvlCB **(*get_lists)();

extern double SimTimeD;

static void print_inst(FILE *fp,struct __vpiScope *scope)
{
  if (scope->scope) {
    print_inst(fp,scope->scope);
    fputs(".",fp);
  } 
  fputs(scope->name,fp);
}

extern "C" int BSgetPWR(vpiHandle argh,SpcIvlCB *cb) 
{
  struct __vpiScope *scp = (struct __vpiScope *)acc_handle_scope(argh);
  for (; scp ; scp = scp->scope) {
    vpiHandle  neth,net_iter;
    eBLtype    bl = BL_REG;
    for (net_iter = vpi_iterate(vpiReg, &scp->base)
           ; bl >= BL_NET ; ((int)bl)--,
         net_iter = vpi_iterate(vpiNet, &scp->base)) {
      if (net_iter) while (neth = vpi_scan(net_iter)) {
        struct __vpiSignal *sgp = (struct __vpiSignal*)neth;
        const char *nm = sgp->id.name;
        int         i;
        if (((i=0,0 == strcmp("vss",nm))    ||
             (i=1,0 == strcmp("vdd",nm))    ||
             (i=0,0 == strcmp("vss__i",nm)) ||
             (i=1,0 == strcmp("vdd__i",nm))) && !cb->pwr[i]) {
          for (SpcIvlCB  *ub = (*get_lists)()[bl]; ub ; ub = ub->next) {
            if (ub->sig == sgp) {
              cb->pwr[i] = ub; goto next;
            }
          }
         next:;
        }
      }
      if (cb->pwr[0] && cb->pwr[1]) {
        if (bs_debug > 0) {
          fprintf(stderr,"Power scope: ");
          print_inst(stderr,scp);
          fprintf(stderr,"\n");
        }
        return 1;
      }
    } 
  }

  return 0;
}

static PLI_INT32 bindsigs_calltf(PLI_BYTE8 *user_data)
{
  if (!get_lists) {
    get_lists = (typeof(get_lists))dlsym(0,"spicenets");
    if (!get_lists) {
      fprintf(stderr,"Bindsigs can't find support function: spicenets\n");
      return -1;
    }
  }
  
  SpcIvlCB **lists = (*get_lists)();
  
  vpiHandle systfref, args_iter, argh, scope;
  struct t_vpi_value argval;
  char *value;
  
  // Obtain a handle to the argument list
  systfref  = vpi_handle(vpiSysTfCall, NULL);
  args_iter = vpi_iterate(vpiArgument, systfref);
  
  // Grab the value of the first argument
  while (argh = vpi_scan(args_iter)) {
    
    struct __vpiSignal *rfp = 0;
    int                 reg = 1;
    switch (argh->vpi_type->type_code) {
    case vpiNet: reg = 0;
    case vpiReg: rfp = (struct __vpiSignal*)argh;
    }

    assert(rfp);

    struct __vpiScope *scope = (struct __vpiScope *)acc_handle_scope(argh),
                      *up;

    if (bs_debug > 0) {
      fprintf(stderr,"Binding ");
      print_inst(stderr,scope);
      fprintf(stderr,".%s",rfp->id.name);
    }

    SpcIvlCB *ub,**ubp = &lists[0];
    
    while (ub = *ubp) {
      const char *spec = ub->spec,
                 *name = rfp->id.name;
      int   hier = 0;
      while (*spec) if ('.' == *spec++) hier++;
      if (hier) {
        int ln = strlen(name),
            ls = 0;
        for (up = scope ;;) {
          for (ls =0; spec-- > ub->spec && '.' != *spec; ls++);
          if (!(ln == ls && (0 == strncmp(spec+1,name,ls)
                             || ('_' == *name && 0 == hier)))) goto next;
          if (hier-- <= 0) break;
          if (name == up->name) {
            up = up->scope;
          } 
          ln = strlen(name = up->name);
        }
        break; 
      } else {
        if (0 == strcmp(ub->spec,name)) {
          break;
        }
      }
     next:
      ubp = &ub->next;
    }

    assert(ub);

    if (bs_debug > 0) {
      fprintf(stderr," to %s\n",ub->spec);
    }

    rfp->ext_bound        = 1 + reg; // eBLtype
    *ubp                  = ub->next;
    ub->next              = lists[rfp->ext_bound];
    lists[rfp->ext_bound] = ub;

    ub->sig  = rfp;

    __vpiScope *scp = rfp->within.scope;
    vpiHandle prmh, prm_iter = vpi_iterate(vpiParameter, &scp->base);
    if (prm_iter) {
      struct __vpiRealVar *parm;
      int    nml  = strlen(rfp->id.name),
             i,p  = sizeof(ub->parm_nm)/sizeof(*ub->parm_nm),
             mode = 0;
      double defaults[p];
      for (i = p; i-- > 0;) defaults[i] = nan("");
      while (prmh = vpi_scan(prm_iter)) {
        char *pnm = vpi_get_str(vpiName,prmh);
        if (0 == strcmp(pnm,"mode")) {
          argval.format = vpiIntVal;
          vpi_get_value(prmh, &argval);
          mode = argval.value.integer;
        } else for (i = p; i-- > 0;) {
          if (0 == strcmp(pnm,SpcIvlCB::parm_nm[i])) {
            argval.format = vpiRealVal;
            vpi_get_value(prmh, &argval);
            defaults[i] = argval.value.real;
          }
        }
      }
      ub->mode = mode;
      ub->set_parms(defaults);
      prm_iter = vpi_iterate(vpiParameter, &scp->base);
      while (prmh = vpi_scan(prm_iter)) {
        char *pnm = vpi_get_str(vpiName,prmh);
        if (0 == strncmp(pnm,rfp->id.name,nml)) {
          const char *sub = pnm + nml;
          if ('_' == *sub++) {
            if (0 == strcmp(sub,"mode")) {
              argval.format = vpiIntVal;
              vpi_get_value(prmh, &argval);
              ub->mode = argval.value.integer;
            } else for (i = p; i-- > 0;) {
              if (0 == strcmp(sub,ub->parm_nm[i])) {
                argval.format = vpiRealVal;
                vpi_get_value(prmh, &argval);
                ub->parms()[i] = argval.value.real;
              }
            }
          }
        }
      }
      set_if_not(ub->lo,        ub->vss);
      set_if_not(ub->hi,        ub->vdd);
      set_if_not(ub->fall,      ub->rise);
      set_if_not(ub->thrshld[0],(ub->hi + ub->lo)/2);
      set_if_not(ub->thrshld[0],0.5);
      set_if_not(ub->thrshld[1],ub->thrshld[0]);
      set_if_not(ub->last_value,ub->init_v);
      set_if_not(ub->prec,      pow(10,scp->time_precision));

      // constrain first timestep
      if (ub->coeffs[1] == ub->init_v) {
	ub->coeffs[2] = SimTimeA + ub->prec/2;
	ub->coeffs[3] = ub->fillEoT(4,ub->init_v);
      } else {
	ub->coeffs[2] = SimTimeA;
	ub->coeffs[4] = SimTimeA + ub->prec/2;
	ub->coeffs[5] = ub->fillEoT(6,ub->init_v);
      }
      if (3 == mode && !((0 == strncmp("vss",rfp->id.name,3) ||
                         (0 == strncmp("vdd",rfp->id.name,3))))) {
        if (!BSgetPWR(argh,ub)) {
            fprintf(stderr,"Power not found for: ");
            print_inst(stderr,scp);
            fprintf(stderr,".%s\n",rfp->id.name);
        } 
      }
    }
    next_arg:;
  }

  return 0;
}

static void bindsigs_register()
{
  s_vpi_systf_data tf_data;
  
  tf_data.type      = vpiSysTask;
  tf_data.tfname    = "$bindsigs";
  tf_data.calltf    = bindsigs_calltf;
  tf_data.compiletf = bindsigs_compiletf;
  tf_data.sizetf    = 0;
  tf_data.user_data = 0;
  vpi_register_systf(&tf_data);
}

static PLI_INT32 sync_out_calltf(PLI_BYTE8 *user_data)
{
  if (!get_lists) return 0;

  vpiHandle systfref, args_iter, argh;
  struct t_vpi_value argval;
  char *value;
  
  // Obtain a handle to the argument list
  systfref  = vpi_handle(vpiSysTfCall, NULL);
  args_iter = vpi_iterate(vpiArgument, systfref);
  
  // Grab the value of the first argument
  argh = vpi_scan(args_iter);

  struct __vpiSignal *rfp = 0;
  int                 reg = 1;
  switch (argh->vpi_type->type_code) {
  case vpiNet: reg = 0;
  case vpiReg: rfp = (struct __vpiSignal*)argh;
  }
  
  assert(rfp);

  argval.format = vpiIntVal;
  vpi_get_value(argh, &argval);

  SpcIvlCB *scan = (*get_lists)()[BL_NET];
  double    v;
  char     *fmt;
  for (; scan; scan = scan->next) {
    if (scan->sig == rfp) {
      fmt = "Syncing net %s=%d\n"; goto found;
    }
  }
  for (scan = get_lists()[BL_REG]; scan; scan = scan->next) {
    if (scan->sig == rfp) {
      fmt = "Syncing reg %s=%d\n"; goto found;
    }
  }

  goto done;  

 found:
  switch (scan->mode) {
  case 3: {
    scan->lo = scan->Pwr(0,"BSgetPWR")->last_value;
    scan->hi = scan->Pwr(1,"BSgetPWR")->last_value;
  }
  case 2:
    if (!isnan(scan->coeffs[4])) { // check on ramp
      int    s   = 0;
      double now = SimTimeA;
      while (now >= scan->coeffs[2+s]) { s += 2; }
      double slew = scan->coeffs[3+s] - scan->coeffs[1+s];
      if (0.0 == slew) {
	slew = scan->coeffs[1+s] - ((scan->hi + scan->lo)/2);
      }
      if ((slew > 0) != argval.value.integer) {
	scan->go2        = argval.value.integer;
	scan->last_error = scan->last_time;
        if (scan->reported = (bs_debug > 0)) {
	  fprintf(stderr,"Warning: PWL/logic mismatch on ");
	  print_inst(stderr,rfp->within.scope);
	  fprintf(stderr,".%s (->%d @ %g)\n",
		         rfp->id.name,argval.value.integer,now);
	  scan->dumpPWL(stderr,now);
	}
      } else if (scan->last_error >= 0.0) {
	double dt = now-scan->last_error;
        if (bs_debug > 0) {
	  fprintf(stderr,"Info: PWL OK ");
	  print_inst(stderr,rfp->within.scope);
	  fprintf(stderr,".%s (->%d @ %g, dt: %g)\n",
		         rfp->id.name,argval.value.integer,now,dt);
	  scan->dumpPWL(stderr,now);        
	} else if (dt > scan->prec/2) {
	  fprintf(stderr,"Warning: PWL/logic mismatch on ");
	  print_inst(stderr,rfp->within.scope);
	  fprintf(stderr,".%s (->%d @ %g for %g)\n",
		         rfp->id.name,argval.value.integer,now-dt,dt);
	}
        scan->last_error = -1;
      }
      break;
    }
  default:
  sync:
    v = scan->lo + argval.value.integer * (scan->hi - scan->lo);
    if (v != scan->coeffs[1]) {
      if (bs_debug > 0) {
        fprintf(stderr,fmt,rfp->id.name,argval.value.integer);
      }
   // scan->coeffs[0] = 0.0;
      scan->coeffs[1] = v;
      scan->coeffs[2] = EndOfTimeD;
      scan->coeffs[3] = v;
    }
    goto done;       
  }

 done:
  // Cleanup and return
  vpi_free_object(args_iter);
  return 0;
}

static void sync_out_register()
{
  s_vpi_systf_data tf_data;

  tf_data.type      = vpiSysTask;
  tf_data.tfname    = "$sync_out";
  tf_data.calltf    = sync_out_calltf;
  tf_data.compiletf = 0;
  tf_data.sizetf    = 0;
  tf_data.user_data = 0;
  vpi_register_systf(&tf_data);
}

static PLI_INT32 sync_in_calltf(PLI_BYTE8 *user_data)
{
  if (!get_lists) return 0;

  vpiHandle systfref, args_iter, argh;
  struct t_vpi_value argval;
  char *value;
  
  // Obtain a handle to the argument list
  systfref  = vpi_handle(vpiSysTfCall, NULL);
  args_iter = vpi_iterate(vpiArgument, systfref);
  
  // Grab the value of the first argument
  argh = vpi_scan(args_iter);

  struct __vpiSignal *rfp = 0;
  int                 reg = 1;
  switch (argh->vpi_type->type_code) {
  case vpiNet: reg = 0;
  case vpiReg: rfp = (struct __vpiSignal*)argh; break;
  }
    
  // if (!reg) rfp = findReg(rfp);

  assert(rfp);

  if (bs_debug > 0) {
    fprintf(stderr,"Syncing %s\n",rfp->id.name);
  }
  // Cleanup and return
  vpi_free_object(args_iter);
  return 0;
}

static void sync_in_register()
{
  s_vpi_systf_data tf_data;

  tf_data.type      = vpiSysTask;
  tf_data.tfname    = "$sync_in";
  tf_data.calltf    = sync_in_calltf;
  tf_data.compiletf = 0;
  tf_data.sizetf    = 0;
  tf_data.user_data = 0;
  vpi_register_systf(&tf_data);
}

extern "C" {
  void (*vlog_startup_routines[])() = {bindsigs_register,
                                       sync_out_register,
                                       sync_in_register,
                                       0};
}
