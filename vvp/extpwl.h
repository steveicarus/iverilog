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

#include <dlfcn.h>

int vvp_main(int,char **);

enum eCBmode { // !!! keep in sync with Spice
    CB_LOAD = 0,
    CB_TRUNC,
    CB_ACCEPT
};

const double EndOfTimeD = 1e99; 

template<int S> 
struct SpiceCallback
{
    typedef double *(*SpiceCB)(SpiceCallback *,double,eCBmode,...);

    double   coeffs[S];
    SpiceCB  eval;
    char    *spec;
    void    *dll_ref;
    void   (*set_active)(void *,void *,double time);

    inline int Slots() {return S;}

    void setCoeffs(double *cof) {
        int t = S;
        while (t-- > 0) coeffs[t] = cof[t];
    }

    inline double fillEoT(int t,double val = 0.0) {
		assert(t < S);
        while (t < S) {
			coeffs[t++] = EndOfTimeD;
			coeffs[t++] = val;
		}
        return val;
    }

    SpiceCallback(double val = 0.0,double dt0 = EndOfTimeD) {
      coeffs[0]  = 0.0;
      coeffs[1]  = val;
      coeffs[2]  = dt0;
      coeffs[3]  = val;
      fillEoT(4,val);
    }

    void dumpPWL(FILE *fp = stderr,double now = 0.0);

    inline void checkPWL(double now = 0.0,double end = 0.0,
                         FILE *fp = stderr) {
      for (int i = 2; i < S; i += 2) {
        if (coeffs[i-2] > coeffs[i]) {
            dumpPWL(fp);
            assert(!"Bad waveform");
        }
        if (end > 0.0 && coeffs[i] >= end) { break; }
      }
    }
};

enum eBLtype {
    BL_UNBOUND = 0,
    BL_NET,
    BL_REG,
    BL_LAST
};

extern "C" void *bindnet(char *spec,char T,int *coeffs,
                         void *,void (*set_active)(void *,void *,double));

#ifdef __vpi_priv_H

#define IVL_PWL_SLOTS 16 /* 8 pairs */

struct SpcIvlCB : SpiceCallback<IVL_PWL_SLOTS> {
    SpcIvlCB    *next,
                *active;       // non-local drive
    __vpiSignal *sig;          // boundary or driven signal

    // Params - keep together (see below)
    double       vss,vdd;
    double       lo,hi;        // drive levels
    double       thrshld[2];   // rising,falling
    double       rise,fall;    // times
    double       init_v;       // initial voltage
    double       prec;         // local precision

    double       last_time,
                 last_value,
                 last_error;
    SpcIvlCB    *pwr[2];       // vss,vdd
    char         set,          // value is known
                 others,       // has other drivers
                 non_local,    // > 0 => not boundary
                 mode;         // 0 - discrete
                               // 1 - ramp over NBA
                               // 2 - sync event
                               // 3 -  + tie to local vss/vdd
	char         go2,          // error fix 
                 reported;

    static const char *parm_nm[]; // SAME ORDER as fields

    inline double *parms() {return &vss;}
    double *set_parms(double *);

    SpcIvlCB(double dt0 = EndOfTimeD,double val = 0.0,
			 const char *nan_tag = "")
     : SpiceCallback <IVL_PWL_SLOTS>(val,dt0) {
      next       = 0;
      sig        = 0;
      active     = 0;
      vss        = nan(nan_tag);
      vdd        = nan(nan_tag);
      hi         = nan(nan_tag);
      lo         = nan(nan_tag);
      thrshld[0] = nan(nan_tag);
      thrshld[1] = nan(nan_tag);
      rise       = nan(nan_tag);
      fall       = nan(nan_tag);
      init_v     = nan(nan_tag);
      prec       = nan(nan_tag);
      set        = 0;
      others     = 0;
      non_local  = 0;
      mode       = 0;
      last_time  = -1;
      last_value = 0;
      last_error = -1;
      pwr[0]     = 0;
      pwr[1]     = 0;
    }

    typedef int (*pwr_fn)(vpiHandle argh,SpcIvlCB *);

    inline SpcIvlCB *Pwr(int i,const char *pfn_nm) {
        if (!pwr[i]) {
            pwr_fn pfn;
            if (pfn_nm) {
                pfn = (pwr_fn)dlsym(0,pfn_nm);
            }
            if (pfn) {
                // (*pfn)(sig,this);
            }
            if (!pwr[i] && non_local) {
                pwr[i] = next->pwr[i];
            }
            assert(pwr[i]);
        } 
        return pwr[i];
    } 

    inline double Rise()      { if (isnan(rise) && non_local > 0) {
                                    rise = next->rise;
                                    assert(!isnan(rise)); }
                                return rise; }

    inline double Fall()      { if (isnan(fall) && non_local > 0) {
                                    fall = next->fall;
                                    assert(!isnan(fall)); }
                                return fall; }

    inline double LastValue() { return non_local > 0 ? next->last_value
                                                     : last_value; }
};

struct SpcDllData {
    char     active;
    double   next_time;
	void   (*activate)(SpcDllData *,SpcIvlCB *,double);
};

void ActivateCB(double time,SpcIvlCB *);

#ifdef DECL__IVL_PARM_NM

inline void set_if_not(double &prm,double val) {
    if (isnan(prm)) prm = val;
}

const char *SpcIvlCB::parm_nm[] = // SAME ORDER as fields above
                           {"vss","vdd",
                            "lo","hi",
                            "thrsh0","thrsh1",
                            "rise","fall",
                            "init_v",
                            "prec"};

double *SpcIvlCB::set_parms(double *from) 
{
   int p = sizeof(parm_nm)/sizeof(*parm_nm);

   while (p--)  parms()[p] = from[p];
}

#endif

extern "C" SpcIvlCB **spicenets();

#endif
