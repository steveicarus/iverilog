
%{
/*
 * Copyright (c) 2000 Stephen Williams (steve@icarus.com)
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
#if !defined(WINNT) && !defined(macintosh)
#ident "$Id: syn-rules.y,v 1.10 2000/11/11 00:03:36 steve Exp $"
#endif

/*
 * This file implements synthesys based on matching threads and
 * converting them to equivilent devices. The trick here is that the
 * proc_match_t functor can be used to scan a process and generate a
 * string of tokens. That string of tokens can then be matched by the
 * rules to determin what kind of device is to be made.
 */

# include  "netlist.h"
# include  "netmisc.h"
# include  "functor.h"
# include  <assert.h>

struct syn_token_t {
      int token;

      NetAssignBase*assign;
      NetAssignMem_*assign_mem;
      NetProcTop*top;
      NetEvWait*evwait;
      NetEvent*event;
      NetExpr*expr;

      syn_token_t*next_;
};
#define YYSTYPE syn_token_t*

static int yylex();
static void yyerror(const char*);
static Design*des_;

static void make_DFF_CE(Design*des, NetProcTop*top, NetEvWait*wclk,
			NetEvent*eclk, NetExpr*cexp, NetAssignBase*asn);
static void make_RAM_CE(Design*des, NetProcTop*top, NetEvWait*wclk,
			NetEvent*eclk, NetExpr*cexp, NetAssignMem_*asn);
static void make_initializer(Design*des, NetProcTop*top, NetAssignBase*asn);

%}

%token S_ALWAYS S_ASSIGN S_ASSIGN_MEM S_ASSIGN_MUX S_ELSE S_EVENT
%token S_EXPR S_IF S_INITIAL

%%

start

  /* These rules match simple DFF devices. Clocked assignments are
     simply implemented as DFF, and a CE is easily expressed with a
     conditional statement. The typical Verilog that get these are:

     always @(posedge CLK) Q = D
     always @(negedge CLK) Q = D

     always @(posedge CLK) if (CE) Q = D;
     always @(negedge CLK) if (CE) Q = D;

     The width of Q and D cause a wide register to be created. The
     code generators generally implement that as an array of
     flip-flops. */

	: S_ALWAYS '@' '(' S_EVENT ')' S_ASSIGN ';'
		{ make_DFF_CE(des_, $1->top, $2->evwait, $4->event,
			      0, $6->assign);
		}

	| S_ALWAYS '@' '(' S_EVENT ')' S_IF S_EXPR S_ASSIGN ';' ';'
		{ make_DFF_CE(des_, $1->top, $2->evwait, $4->event,
			      $7->expr, $8->assign);
		}

  /* Unconditional assignments in initial blocks should be made into
     initializers wherever possible. */

	| S_INITIAL S_ASSIGN
		{ make_initializer(des_, $1->top, $2->assign);
		}


  /* These rules match RAM devices. They are similar to DFF, except
     that there is an index for the word. The typical Verilog that get
     these are:

     always @(posedge CLK) M[a] = D
     always @(negedge CLK) M[a] = D

     always @(posedge CLK) if (CE) M[a] = D;
     always @(negedge CLK) if (CE) M[a] = D;

     The width of Q and D cause a wide register to be created. The
     code generators generally implement that as an array of
     flip-flops. */

	| S_ALWAYS '@' '(' S_EVENT ')' S_ASSIGN_MEM ';'
		{ make_RAM_CE(des_, $1->top, $2->evwait, $4->event,
			      0, $6->assign_mem);
		}

	| S_ALWAYS '@' '(' S_EVENT ')' S_IF S_EXPR S_ASSIGN_MEM ';' ';'
		{ make_RAM_CE(des_, $1->top, $2->evwait, $4->event,
			      $7->expr, $8->assign_mem);
		}

	;

%%


  /* Various actions. */
static void make_DFF_CE(Design*des, NetProcTop*top, NetEvWait*wclk,
			NetEvent*eclk, NetExpr*cexp, NetAssignBase*asn)
{
      NetEvProbe*pclk = eclk->probe(0);
      NetESignal*d = dynamic_cast<NetESignal*> (asn->rval());
      NetNet*ce = cexp? cexp->synthesize(des) : 0;

      assert(d);

      NetFF*ff = new NetFF(top->scope(), asn->l_val(0)->name(),
			   asn->l_val(0)->pin_count());

      for (unsigned idx = 0 ;  idx < ff->width() ;  idx += 1) {
	    connect(ff->pin_Data(idx), d->pin(idx));
	    connect(ff->pin_Q(idx), asn->l_val(0)->pin(idx));
      }

      connect(ff->pin_Clock(), pclk->pin(0));
      if (ce) connect(ff->pin_Enable(), ce->pin(0));

      ff->attribute("LPM_FFType", "DFF");
      if (pclk->edge() == NetEvProbe::NEGEDGE)
	    ff->attribute("Clock:LPM_Polarity", "INVERT");

      des->add_node(ff);
      des->delete_process(top);
}

static void make_RAM_CE(Design*des, NetProcTop*top, NetEvWait*wclk,
			NetEvent*eclk, NetExpr*cexp, NetAssignMem_*asn)
{
      NetMemory*mem = asn->memory();
      NetExpr*adr_e = asn->index();

      NetNet*adr = adr_e->synthesize(des);
      assert(adr);

      NetEvProbe*pclk = eclk->probe(0);
      NetESignal*d = dynamic_cast<NetESignal*> (asn->rval());
      NetNet*ce = cexp? cexp->synthesize(des) : 0;

      assert(d);

      NetRamDq*ram = new NetRamDq(des->local_symbol(mem->name()), mem,
				  adr->pin_count());

      for (unsigned idx = 0 ;  idx < adr->pin_count() ;  idx += 1)
	    connect(adr->pin(idx), ram->pin_Address(idx));

      for (unsigned idx = 0 ;  idx < ram->width() ;  idx += 1)
	    connect(ram->pin_Data(idx), d->pin(idx));

      if (ce) connect(ram->pin_WE(), ce->pin(0));

      assert(pclk->edge() == NetEvProbe::POSEDGE);
      connect(ram->pin_InClock(), pclk->pin(0));

      ram->absorb_partners();

      des->add_node(ram);
      des->delete_process(top);
}

/*
 * An assignment in an initial statement is the same as giving the
 * nexus an initial value. For synthesized netlists, we can just set
 * the initial value for the link and get rid of the assignment
 * process.
 */
static void make_initializer(Design*des, NetProcTop*top, NetAssignBase*asn)
{
      NetESignal*rsig = dynamic_cast<NetESignal*> (asn->rval());
      assert(rsig);

      for (unsigned idx = 0 ;  idx < asn->l_val(0)->pin_count() ;  idx += 1) {

	    verinum::V bit = driven_value(rsig->pin(idx));

	    Nexus*nex = asn->l_val(0)->pin(idx).nexus();
	    for (Link*cur = nex->first_nlink()
		       ;  cur ;  cur = cur->next_nlink()) {

		  if (NetNet*net = dynamic_cast<NetNet*> (cur->get_obj()))
			cur->set_init(bit);

	    }
      }

      des->delete_process(top);
}

static syn_token_t*first_ = 0;
static syn_token_t*last_ = 0;
static syn_token_t*ptr_ = 0;

/*
 * The match class is used to take a process and turn it into a stream
 * of tokens. This stream is used by the yylex function to feed tokens
 * to the parser.
 */
struct tokenize : public proc_match_t {
      tokenize() { }
      ~tokenize() { }

      int assign(NetAssign*dev)
      {
	    syn_token_t*cur;
	    cur = new syn_token_t;
	    cur->token = dev->l_val(0)->bmux() ? S_ASSIGN_MUX : S_ASSIGN;
	    cur->assign = dev;
	    cur->next_ = 0;
	    last_->next_ = cur;
	    last_ = cur;
	    return 0;
      }

      int assign_nb(NetAssignNB*dev)
      {
	    syn_token_t*cur;
	    cur = new syn_token_t;
	    cur->token = dev->l_val(0)->bmux() ? S_ASSIGN_MUX : S_ASSIGN;
	    cur->assign = dev;
	    cur->next_ = 0;
	    last_->next_ = cur;
	    last_ = cur;
	    return 0;
      }

      int assign_mem(NetAssignMem*dev)
      {
	    syn_token_t*cur;
	    cur = new syn_token_t;
	    cur->token = S_ASSIGN_MEM;
	    cur->assign_mem = dev;
	    cur->next_ = 0;
	    last_->next_ = cur;
	    last_ = cur;
	    return 0;
      }

      int assign_mem_nb(NetAssignMemNB*dev)
      {
	    syn_token_t*cur;
	    cur = new syn_token_t;
	    cur->token = S_ASSIGN_MEM;
	    cur->assign_mem = dev;
	    cur->next_ = 0;
	    last_->next_ = cur;
	    last_ = cur;
	    return 0;
      }

      int condit(NetCondit*dev)
      {
	    syn_token_t*cur;
	    
	    cur = new syn_token_t;
	    cur->token = S_IF;
	    cur->next_ = 0;
	    last_->next_ = cur;
	    last_ = cur;

	    cur = new syn_token_t;
	    cur->token = S_EXPR;
	    cur->expr = dev->expr();
	    cur->next_ = 0;
	    last_->next_ = cur;
	    last_ = cur;

	    dev -> if_clause() -> match_proc(this);

	    if (dev->else_clause()) {
		  cur = new syn_token_t;
		  cur->token = S_ELSE;
		  cur->next_ = 0;
		  last_->next_ = cur;
		  last_ = cur;

		  dev -> else_clause() -> match_proc(this);
	    }

	    cur = new syn_token_t;
	    cur->token = ';';
	    cur->next_ = 0;
	    last_->next_ = cur;
	    last_ = cur;
	    return 0;
      }

      int event_wait(NetEvWait*dev)
      {
	    syn_token_t*cur;
	    
	    cur = new syn_token_t;
	    cur->token = '@';
	    cur->evwait = dev;
	    cur->next_ = 0;
	    last_->next_ = cur;
	    last_ = cur;

	    cur = new syn_token_t;
	    cur->token = '(';
	    cur->next_ = 0;
	    last_->next_ = cur;
	    last_ = cur;

	    for (unsigned idx = 0;  idx < dev->nevents(); idx += 1) {
		  cur = new syn_token_t;
		  cur->token = S_EVENT;
		  cur->event = dev->event(idx);
		  cur->next_ = 0;
		  last_->next_ = cur;
		  last_ = cur;
	    }

	    cur = new syn_token_t;
	    cur->token = ')';
	    cur->next_ = 0;
	    last_->next_ = cur;
	    last_ = cur;

	    dev -> statement() -> match_proc(this);

	    cur = new syn_token_t;
	    cur->token = ';';
	    cur->next_ = 0;
	    last_->next_ = cur;
	    last_ = cur;
	    return 0;
      }
};

static void syn_start_process(NetProcTop*t)
{
      first_ = new syn_token_t;
      last_ = first_;
      ptr_ = first_;

      first_->token = (t->type() == NetProcTop::KALWAYS)? S_ALWAYS : S_INITIAL;
      first_->top = t;
      first_->next_ = 0;

      tokenize go;
      t -> statement() -> match_proc(&go);
}

static void syn_done_process()
{
      while (first_) {
	    syn_token_t*cur = first_;
	    first_ = cur->next_;
	    delete cur;
      }
}

static int yylex()
{
      if (ptr_ == 0) {
	    yylval = 0;
	    return EOF;
      }

      yylval = ptr_;
      ptr_ = ptr_->next_;
      return yylval->token;
}

struct syn_rules_f  : public functor_t {
      ~syn_rules_f() { }

      void process(class Design*des, class NetProcTop*top)
      {
	    syn_start_process(top);
	    yyparse();
	    syn_done_process();
      }
};

void syn_rules(Design*d)
{
      des_ = d;
      syn_rules_f obj;
      des_->functor(&obj);
}

static void yyerror(const char*)
{
}
