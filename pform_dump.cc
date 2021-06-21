/*
 * Copyright (c) 1998-2021 Stephen Williams (steve@icarus.com)
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
 *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

# include "config.h"

/*
 * This file provides the pform_dump function, that dumps the module
 * passed as a parameter. The dump is as much as possible in Verilog
 * syntax, so that a human can tell that it really does describe the
 * module in question.
 */
# include  "pform.h"
# include  "PClass.h"
# include  "PEvent.h"
# include  "PGenerate.h"
# include  "PPackage.h"
# include  "PSpec.h"
# include  "PTask.h"
# include  "discipline.h"
# include  "ivl_target_priv.h"
# include  <iostream>
# include  <iomanip>
# include  <typeinfo>

ostream& operator << (ostream&out, const PExpr&obj)
{
      obj.dump(out);
      return out;
}

ostream& operator << (ostream&out, const PEventStatement&obj)
{
      obj.dump_inline(out);
      return out;
}

ostream& operator << (ostream&o, const PDelays&d)
{
      d.dump_delays(o);
      return o;
}

ostream& operator<< (ostream&out, const index_component_t&that)
{
      out << "[";
      switch (that.sel) {
	  case index_component_t::SEL_BIT:
	    out << *that.msb;
	    break;
	  case index_component_t::SEL_BIT_LAST:
	    out << "$";
	    break;
	  case index_component_t::SEL_PART:
	    out << *that.msb << ":" << *that.lsb;
	    break;
	  case index_component_t::SEL_IDX_UP:
	    out << *that.msb << "+:" << *that.lsb;
	    break;
	  case index_component_t::SEL_IDX_DO:
	    out << *that.msb << "-:" << *that.lsb;
	    break;
	  default:
	    out << "???";
	    break;
      }
      out << "]";
      return out;
}

ostream& operator<< (ostream&out, const name_component_t&that)
{
      out << that.name.str();

      typedef std::list<index_component_t>::const_iterator index_it_t;
      for (index_it_t idx = that.index.begin()
		 ; idx != that.index.end() ; ++ idx ) {

	    out << *idx;
      }
      return out;
}

ostream& operator<< (ostream&o, const pform_name_t&that)
{
      pform_name_t::const_iterator cur;
      if (that.empty()) {
	    o << "<nil>";
	    return o;
      }

      cur = that.begin();
      o << *cur;

      ++ cur;
      while (cur != that.end()) {
	    o << "." << *cur;
	    ++ cur;
      }

      return o;
}

std::ostream& operator << (std::ostream&out, ivl_process_type_t pt)
{
      switch (pt) {
	  case IVL_PR_INITIAL:
	    out << "initial";
	    break;
	  case IVL_PR_ALWAYS:
	    out << "always";
	    break;
	  case IVL_PR_ALWAYS_COMB:
	    out << "always_comb";
	    break;
	  case IVL_PR_ALWAYS_FF:
	    out << "always_ff";
	    break;
	  case IVL_PR_ALWAYS_LATCH:
	    out << "always_latch";
	    break;
	  case IVL_PR_FINAL:
	    out << "final";
	    break;
      }
      return out;
}

std::ostream& operator << (std::ostream&out, ivl_dis_domain_t dom)
{
      switch (dom) {
	  case IVL_DIS_NONE:
	    out << "no-domain";
	    break;
	  case IVL_DIS_DISCRETE:
	    out << "discrete";
	    break;
	  case IVL_DIS_CONTINUOUS:
	    out << "continuous";
	    break;
	  default:
	    assert(0);
	    break;
      }
      return out;
}

void data_type_t::pform_dump(ostream&out, unsigned indent) const
{
      out << setw(indent) << "" << typeid(*this).name() << endl;
}

ostream& data_type_t::debug_dump(ostream&out) const
{
      out << typeid(*this).name();
      return out;
}

ostream& atom2_type_t::debug_dump(ostream&out) const
{
      if (signed_flag)
	    out << "signed-";
      else
	    out << "unsigned-";
      out << "int(" << type_code << ")";
      return out;
}

void void_type_t::pform_dump(ostream&out, unsigned indent) const
{
      out << setw(indent) << "" << "void" << endl;
}

void parray_type_t::pform_dump(ostream&out, unsigned indent) const
{
      out << setw(indent) << "" << "Packed array " << "[...]"
	  << " of:" << endl;
      base_type->pform_dump(out, indent+4);
}

ostream& real_type_t::debug_dump(ostream&out) const
{
      switch (type_code_) {
	  case REAL:
	    out << "real";
	    break;
	  case SHORTREAL:
	    out << "shortreal";
	    break;
      }
      return out;
}

void struct_type_t::pform_dump(ostream&out, unsigned indent) const
{
      out << setw(indent) << "" << "Struct " << (packed_flag?"packed":"unpacked")
	  << " with " << (members.get()==0? 0 : members->size()) << " members" << endl;
      if (members.get()==0)
	    return;

      for (list<struct_member_t*>::iterator cur = members->begin()
		 ; cur != members->end() ; ++ cur) {
	    struct_member_t*curp = *cur;
	    curp->pform_dump(out, indent+4);
      }
}

void uarray_type_t::pform_dump(ostream&out, unsigned indent) const
{
      out << setw(indent) << "" << "Unpacked array " << "[...]"
	  << " of:" << endl;
      base_type->pform_dump(out, indent+4);
}

void vector_type_t::pform_dump(ostream&fd, unsigned indent) const
{
      fd << setw(indent) << "" << "vector of " << base_type;
      if (pdims.get()) {
	    for (list<pform_range_t>::iterator cur = pdims->begin()
		       ; cur != pdims->end() ; ++cur) {
		  fd << "[";
		  if (cur->first)  fd << *(cur->first);
		  if (cur->second) fd << ":" << *(cur->second);
		  fd << "]";
	    }
      }
      fd << endl;
}

ostream& vector_type_t::debug_dump(ostream&fd) const
{
      if (signed_flag)
	    fd << "signed ";
      if (!pdims.get()) {
	    fd << "/* vector_type_t nil */";
	    return fd;
      }

      for (list<pform_range_t>::iterator cur = pdims->begin()
		 ; cur != pdims->end() ; ++cur) {
	    fd << "[";
	    if (cur->first)  fd << *(cur->first);
	    if (cur->second) fd << ":" << *(cur->second);
	    fd << "]";
      }
      return fd;
}

void class_type_t::pform_dump(ostream&out, unsigned indent) const
{
      out << setw(indent) << "" << "class " << name;

      if (base_type) out << " extends <type>";
      if (! base_args.empty()) {
	    out << " (";
	    for (list<PExpr*>::const_iterator cur = base_args.begin()
		       ; cur != base_args.end() ; ++cur) {
		  const PExpr*curp = *cur;
		  if (cur != base_args.begin())
			out << ", ";
		  curp->dump(out);
	    }
	    out << ")";
      }

      out << " {";
      for (map<perm_string,prop_info_t>::const_iterator cur = properties.begin()
		 ; cur != properties.end() ; ++cur) {
	    out << " " << cur->first;
      }

      out << " }" << endl;

      if (base_type) base_type->pform_dump(out, indent+4);
}

void class_type_t::pform_dump_init(ostream&out, unsigned indent) const
{
      for (vector<Statement*>::const_iterator cur = initialize.begin()
		 ; cur != initialize.end() ; ++cur) {
	    Statement*curp = *cur;
	    curp->dump(out,indent+4);
      }
}

void struct_member_t::pform_dump(ostream&out, unsigned indent) const
{
      out << setw(indent) << "" << (type.get()? typeid(*type).name() : "<nil type>");
      for (list<decl_assignment_t*>::iterator cur = names->begin()
		 ; cur != names->end() ; ++cur) {
	    decl_assignment_t*curp = *cur;
	    out << " " << curp->name;
      }
      out << ";" << endl;
}

static void dump_attributes_map(ostream&out,
				const map<perm_string,PExpr*>&attributes,
				int ind)
{
      for (map<perm_string,PExpr*>::const_iterator idx = attributes.begin()
		 ; idx != attributes.end() ; ++ idx ) {

	    out << setw(ind) << "" << "(* " << (*idx).first;
	    if ((*idx).second) {
		  out << " = " << *(*idx).second;
	    }
	    out << " *)" << endl;
      }
}

void PExpr::dump(ostream&out) const
{
      out << typeid(*this).name();
}

void PEAssignPattern::dump(ostream&out) const
{
      out << "'{";
      if (parms_.size() > 0) {
	    parms_[0]->dump(out);
	    for (size_t idx = 1 ; idx < parms_.size() ; idx += 1) {
		  out << ", ";
		  parms_[idx]->dump(out);
	    }
      }
      out << "}";
}

void PEConcat::dump(ostream&out) const
{
      if (repeat_)
	    out << "{" << *repeat_;

      if (parms_.empty()) {
	    out << "{}";
	    return;
      }

      out << "{";
      if (parms_[0]) out << *parms_[0];
      for (unsigned idx = 1 ;  idx < parms_.size() ;  idx += 1) {
	    out << ", ";
	    if (parms_[idx]) out << *parms_[idx];
      }

      out << "}";

      if (repeat_) out << "}";
}

void PECallFunction::dump(ostream &out) const
{
      if (package_) out << package_->pscope_name() << "::";

      out << path_ << "(";

      if (! parms_.empty()) {
	    if (parms_[0]) parms_[0]->dump(out);
	    for (unsigned idx = 1; idx < parms_.size(); ++idx) {
		  out << ", ";
		  if (parms_[idx]) parms_[idx]->dump(out);
	    }
      }
      out << ")";
}

void PECastSize::dump(ostream &out) const
{
      out << *size_ << "'(";
      base_->dump(out);
      out << ")";
}

void PECastType::dump(ostream &out) const
{
      target_->pform_dump(out, 0);
      out << "'(";
      base_->dump(out);
      out << ")";
}

void PEEvent::dump(ostream&out) const
{
      switch (type_) {
	  case PEEvent::ANYEDGE:
	    break;
	  case PEEvent::POSEDGE:
	    out << "posedge ";
	    break;
	  case PEEvent::NEGEDGE:
	    out << "negedge ";
	    break;
	  case PEEvent::EDGE:
	    out << "edge ";
	    break;
	  case PEEvent::POSITIVE:
	    out << "positive ";
	    break;
      }
      out << *expr_;

}

void PEFNumber::dump(ostream &out) const
{
      out << value();
}

void PENewArray::dump(ostream&out) const
{
      out << "new [" << *size_ << "]";
      if (init_)
	    out << "(" << *init_ << ")";
}

void PENewClass::dump(ostream&out) const
{
      out << "class_new(";
      if (parms_.size() > 0) {
	    parms_[0]->dump(out);
	    for (size_t idx = 1 ; idx < parms_.size() ; idx += 1) {
		  out << ", ";
		  if (parms_[idx]) parms_[idx]->dump(out);
	    }
      }
      out << ")";
}

void PENewCopy::dump(ostream&out) const
{
      out << "copy_new(";
      src_->dump(out);
      out << ")";
}

void PENull::dump(ostream&out) const
{
      out << "null";
}

void PENumber::dump(ostream&out) const
{
      out << value();
}

void PEIdent::dump(ostream&out) const
{
      if (package_)
	    out << package_->pscope_name() << "::";
      out << path_;
}

void PEString::dump(ostream&out) const
{
      out << "\"" << text_ << "\"";
}

void PETernary::dump(ostream&out) const
{
      out << "(" << *expr_ << ")?(" << *tru_ << "):(" << *fal_ << ")";
}

void PETypename::dump(ostream&fd) const
{
      fd << "<type>";
}

void PEUnary::dump(ostream&out) const
{
      switch (op_) {
	  case 'm':
	    out << "abs";
	    break;
	  default:
	    out << op_;
	    break;
      }
      out << "(" << *expr_ << ")";
}

void PEBinary::dump(ostream&out) const
{
	/* Handle some special cases, where the operators are written
	   in function notation. */
      if (op_ == 'm') {
	    out << "min(" << *left_ << "," << *right_ << ")";
	    return;
      }
      if (op_ == 'M') {
	    out << "min(" << *left_ << "," << *right_ << ")";
	    return;
      }

      if (left_)
	    out << "(" << *left_ << ")";
      else
	    out << "(<nil>)";
      switch (op_) {
	  case 'a':
	    out << "&&";
	    break;
	  case 'e':
	    out << "==";
	    break;
	  case 'E':
	    out << "===";
	    break;
	  case 'l':
	    out << "<<";
	    break;
	  case 'L':
	    out << "<=";
	    break;
	  case 'n':
	    out << "!=";
	    break;
	  case 'N':
	    out << "!==";
	    break;
	  case 'p':
	    out << "**";
	    break;
	  case 'R':
	    out << ">>>";
	    break;
	  case 'r':
	    out << ">>";
	    break;
	  default:
	    out << op_;
	    break;
      }
      if (right_)
	    out << "(" << *right_ << ")";
      else
	    out << "(<nil>)";
}


void PWire::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << type_;

      switch (port_type_) {
	  case NetNet::PIMPLICIT:
	    out << " implicit input";
	    break;
	  case NetNet::PINPUT:
	    out << " input";
	    break;
	  case NetNet::POUTPUT:
	    out << " output";
	    break;
	  case NetNet::PINOUT:
	    out << " inout";
	    break;
	  case NetNet::PREF:
	    out << " ref";
	    break;
	  case NetNet::NOT_A_PORT:
	    break;
      }

      out << " " << data_type_;

      if (signed_) {
	    out << " signed";
      }
      if (get_isint()) {
	    out << " integer";
      }
      if (get_scalar()) {
	    out << " scalar";
      }
      if (set_data_type_) {
	    out << " set_data_type_=" << *set_data_type_;
      }

      if (discipline_) {
	    out << " discipline<" << discipline_->name() << ">";
      }

      if (port_set_) {
	    if (port_.empty()) {
		  out << " port<scalar>";
	    } else {
		  out << " port";
		  for (list<pform_range_t>::const_iterator cur = port_.begin()
			     ; cur != port_.end() ; ++cur) {
			out << "[";
			if (cur->first)  out << *cur->first;
			if (cur->second) out << ":" << *cur->second;
			out << "]";
		  }
	    }
      }
      if (net_set_) {
	    if (net_.empty()) {
		  out << " net<scalar>";
	    } else {
		  out << " net";
		  for (list<pform_range_t>::const_iterator cur = net_.begin()
			     ; cur != net_.end() ; ++cur) {
			out << "[";
			if (cur->first)  out << *cur->first;
			if (cur->second) out << ":" << *cur->second;
			out << "]";
		  }
	    }
      }

      out << " " << name_;

	// If the wire has unpacked indices, dump them.
      for (list<pform_range_t>::const_iterator cur = unpacked_.begin()
		 ; cur != unpacked_.end() ; ++cur) {
	    out << "[";
	    if (cur->first) out << *cur->first;
	    if (cur->second) out << ":" << *cur->second;
	    out << "]";
      }

      out << ";" << endl;
      if (set_data_type_) {
	    set_data_type_->pform_dump(out, 8);
      }

      dump_attributes_map(out, attributes, 8);
}

void PGate::dump_pins(ostream&out) const
{
      if (pin_count()) {
	    if (pin(0)) out << *pin(0);

	    for (unsigned idx = 1 ;  idx < pin_count() ;  idx += 1) {
		  out << ", ";
		  if (pin(idx)) out << *pin(idx);
	    }
      }
}

void PDelays::dump_delays(ostream&out) const
{
      if (delay_[0] && delay_[1] && delay_[2])
	    out << "#(" << *delay_[0] << "," << *delay_[1] << "," <<
		  *delay_[2] << ")";
      else if (delay_[0] && delay_[1])
	    out << "#(" << *delay_[0] << "," << *delay_[1] << ")";
      else if (delay_[0])
	    out << "#" << *delay_[0];
      else
	    out << "#0";

}

void PGate::dump_delays(ostream&out) const
{
      delay_.dump_delays(out);
}

void PGate::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << typeid(*this).name() << " ";
      delay_.dump_delays(out);
      out << " " << get_name() << "(";
      dump_pins(out);
      out << ");" << endl;

}

void PGAssign::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "";
      out << "assign (" << strength0() << "0 " << strength1() << "1) ";
      dump_delays(out);
      out << " " << *pin(0) << " = " << *pin(1) << ";" << endl;
}

void PGBuiltin::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "";
      switch (type()) {
	  case PGBuiltin::BUFIF0:
	    out << "bufif0 ";
	    break;
	  case PGBuiltin::BUFIF1:
	    out << "bufif1 ";
	    break;
	  case PGBuiltin::NOTIF0:
	    out << "notif0 ";
	    break;
	  case PGBuiltin::NOTIF1:
	    out << "notif1 ";
	    break;
	  case PGBuiltin::NAND:
	    out << "nand ";
	    break;
	  case PGBuiltin::NMOS:
	    out << "nmos ";
	    break;
	  case PGBuiltin::RNMOS:
	    out << "rnmos ";
	    break;
	  case PGBuiltin::RPMOS:
	    out << "rpmos ";
	    break;
	  case PGBuiltin::PMOS:
	    out << "pmos ";
	    break;
	  case PGBuiltin::RCMOS:
	    out << "rcmos ";
	    break;
	  case PGBuiltin::CMOS:
	    out << "cmos ";
	    break;
	  default:
	    out << "builtin gate ";
      }

      out << "(" << strength0() << "0 " << strength1() << "1) ";
      dump_delays(out);
      out << " " << get_name();

      if (msb_) {
	    out << " [" << *msb_ << ":" << *lsb_ << "]";
      }

      out << "(";
      dump_pins(out);
      out << ");" << endl;
}

void PGModule::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << type_ << " ";

	// If parameters are overridden by order, dump them.
      if (overrides_ && (! overrides_->empty())) {
	    assert(parms_ == 0);
            out << "#(";

	    list<PExpr*>::const_iterator idx = overrides_->begin();

	    if (*idx == 0)
		  out << "<nil>";
	    else
		  out << *idx;
	    for ( ;  idx != overrides_->end() ;  ++ idx) {
	          out << "," << *idx;
	    }
	    out << ") ";
      }

	// If parameters are overridden by name, dump them.
      if (parms_) {
	    assert(overrides_ == 0);
	    out << "#(";
	    for (unsigned idx = 0 ;  idx < nparms_ ;  idx += 1) {
                  if (idx > 0) out << ", ";
		  out << "." << parms_[idx].name << "(";
                  if (parms_[idx].parm) out << *parms_[idx].parm;
                  out << ")";
	    }
	    out << ") ";
      }

      out << get_name();

	// If the module is arrayed, print the index expressions.
      if (msb_ || lsb_) {
	    out << "[";
	    if (msb_) out << *msb_;
	    out << ":";
	    if (lsb_) out << *lsb_;
	    out << "]";
      }

      out << "(";
      if (pins_) {
	    out << "." << pins_[0].name << "(";
	    if (pins_[0].parm) out << *pins_[0].parm;
	    out << ")";
	    for (unsigned idx = 1 ;  idx < npins_ ;  idx += 1) {
		  out << ", ." << pins_[idx].name << "(";
		  if (pins_[idx].parm)
			out << *pins_[idx].parm;
		  out << ")";
	    }
      } else {
	    dump_pins(out);
      }
      out << ");" << endl;
      dump_attributes_map(out, attributes, 8);
}

void Statement::dump(ostream&out, unsigned ind) const
{
	/* I give up. I don't know what type this statement is,
	   so just print the C++ typeid and let the user figure
	   it out. */
      out << setw(ind) << "";
      out << "/* " << get_fileline() << ": " << typeid(*this).name()
	  << " */ ;" << endl;
      dump_attributes_map(out, attributes, ind+2);
}

void AContrib::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "";
      out << *lval_ << " <+ " << *rval_
	  << "; /* " << get_fileline() << " */"
	  << endl;
}

void PAssign::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "";
      if (lval()) out << *lval();
      else out << "<dummy>";
      out << " = ";
      if (delay_) out << "#" << *delay_ << " ";
      if (count_) out << "repeat(" << *count_ << ") ";
      if (event_) out << *event_ << " ";
      PExpr*rexpr = rval();
      if (rexpr) out << *rval() << ";";
      else out << "<no rval>;";
      out << "  /* " << get_fileline() << " */" << endl;
}

void PAssignNB::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << *lval() << " <= ";
      if (delay_) out << "#" << *delay_ << " ";
      if (count_) out << "repeat(" << *count_ << ") ";
      if (event_) out << *event_ << " ";
      out << *rval() << ";" << "  /* " << get_fileline() << " */" << endl;
}

void PBlock::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "begin";
      if (pscope_name() != 0)
	    out << " : " << pscope_name();
      out << endl;

      if (pscope_name() != 0) {
            dump_parameters_(out, ind+2);

            dump_localparams_(out, ind+2);

            dump_events_(out, ind+2);

	    dump_wires_(out, ind+2);

	    dump_var_inits_(out, ind+2);
      }

      for (unsigned idx = 0 ;  idx < list_.size() ;  idx += 1) {
	    if (list_[idx])
		  list_[idx]->dump(out, ind+2);
	    else
		  out << setw(ind+2) << "" << "/* NOOP */ ;" << endl;
      }

      out << setw(ind) << "" << "end" << endl;
}

void PCallTask::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << path_;

      if (! parms_.empty()) {
	    out << "(";
	    if (parms_[0])
		  out << *parms_[0];

	    for (unsigned idx = 1 ;  idx < parms_.size() ;  idx += 1) {
		  out << ", ";
		  if (parms_[idx])
			out << *parms_[idx];
	    }
	    out << ")";
      }

      out << "; /* " << get_fileline() << " */" << endl;
}

void PCase::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "";
      switch (quality_) {
	  case IVL_CASE_QUALITY_BASIC:
	    break;
	  case IVL_CASE_QUALITY_UNIQUE:
	    out << "unique ";
	    break;
	  case IVL_CASE_QUALITY_UNIQUE0:
	    out << "unique0 ";
	    break;
	  case IVL_CASE_QUALITY_PRIORITY:
	    out << "priority ";
	    break;
      }
      switch (type_) {
	  case NetCase::EQ:
	    out << "case";
	    break;
	  case NetCase::EQX:
	    out << "casex";
	    break;
	  case NetCase::EQZ:
	    out << "casez";
	    break;
      }
      out << " (" << *expr_ << ") /* " << get_fileline() << " */" << endl;
      dump_attributes_map(out, attributes, ind+2);

      for (unsigned idx = 0 ;  idx < items_->count() ;  idx += 1) {
	    PCase::Item*cur = (*items_)[idx];

	    if (cur->expr.empty()) {
		  out << setw(ind+2) << "" << "default:";

	    } else {
		  list<PExpr*>::iterator idx_exp = cur->expr.begin();
		  out << setw(ind+2) << "";
		  (*idx_exp)->dump(out);

		  for (++idx_exp ; idx_exp != cur->expr.end() ; ++idx_exp) {
			out << ", ";
			(*idx_exp)->dump(out);
		  }

		  out << ":";
	    }

	    if (cur->stat) {
		  out << endl;
		  cur->stat->dump(out, ind+6);
	    } else {
		  out << " ;" << endl;
	    }
      }

      out << setw(ind) << "" << "endcase" << endl;
}

void PChainConstructor::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "super.new(";
      if (parms_.size() > 0) {
	    if (parms_[0]) out << *parms_[0];
      }
      for (size_t idx = 1 ; idx < parms_.size() ; idx += 1) {
	    out << ", ";
	    if (parms_[idx]) out << *parms_[idx];
      }
      out << ");" << endl;
}

void PCondit::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "if (" << *expr_ << ")" << endl;
      if (if_)
	    if_->dump(out, ind+3);
      else
	    out << setw(ind) << ";" << endl;
      if (else_) {
	    out << setw(ind) << "" << "else" << endl;
	    else_->dump(out, ind+3);
      }
}

void PCAssign::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "assign " << *lval_ << " = " << *expr_
	  << "; /* " << get_fileline() << " */" << endl;
}

void PDeassign::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "deassign " << *lval_ << "; /* "
	  << get_fileline() << " */" << endl;
}

void PDelayStatement::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "#" << *delay_ << " /* " <<
	    get_fileline() << " */";
      if (statement_) {
	    out << endl;
	    statement_->dump(out, ind+2);
      } else {
	    out << " /* noop */;" << endl;
      }
}

void PDisable::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "disable ";
      if (scope_.empty()) out << scope_;
      else out << "fork";
      out << "; /* " << get_fileline() << " */" << endl;
}

void PDoWhile::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "do" << endl;
      if (statement_)
	    statement_->dump(out, ind+3);
      else
	    out << setw(ind+3) << "" << "/* NOOP */" << endl;
      out << setw(ind) << "" << "while (" << *cond_ << ");" << endl;
}

void PEventStatement::dump(ostream&out, unsigned ind) const
{
      if (expr_.count() == 0) {
	    out << setw(ind) << "" << "@* ";

      } else if ((expr_.count() == 1) && (expr_[0] == 0)) {
	    out << setw(ind) << "" << "wait fork ";

      } else {
	    out << setw(ind) << "" << "@(" << *(expr_[0]);
	    if (expr_.count() > 1)
		  for (unsigned idx = 1 ;  idx < expr_.count() ;  idx += 1)
			out << " or " << *(expr_[idx]);

	    out << ")";
      }

      if (statement_) {
	    out << endl;
	    statement_->dump(out, ind+2);
      } else {
	    out << " ;" << endl;
      }
}

void PEventStatement::dump_inline(ostream&out) const
{
      assert(statement_ == 0);

      if (expr_.count() == 0) {
	    out << "@* ";

      } else {
	    out << "@(" << *(expr_[0]);
	    if (expr_.count() > 1)
		  for (unsigned idx = 1 ;  idx < expr_.count() ;  idx += 1)
			out << " or " << *(expr_[idx]);

	    out << ")";
      }
}

void PForce::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "force " << *lval_ << " = " << *expr_
	  << "; /* " << get_fileline() << " */" << endl;
}

void PForeach::dump(ostream&fd, unsigned ind) const
{
      fd << setw(ind) << "" << "foreach "
	 << "variable=" << array_var_
	 << ", indices=[";
      for (size_t idx = 0 ; idx < index_vars_.size() ; idx += 1) {
	    if (idx > 0) fd << ",";
	    fd << index_vars_[idx];
      }

      fd << "] /* " << get_fileline() << " */" << endl;
      if (statement_)
	    statement_->dump(fd, ind+3);
      else
	    fd << setw(ind+3) << "" << "/* NOOP */" << endl;
}

void PForever::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "forever /* " << get_fileline() << " */" << endl;
      if (statement_)
	    statement_->dump(out, ind+3);
      else
	    out << setw(ind+3) << "" << "/* NOOP */" << endl;
}

void PForStatement::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "for (" << *name1_ << " = " << *expr1_
	  << "; " << *cond_ << "; <for_step>)" << endl;
      step_->dump(out, ind+6);
      if (statement_)
	    statement_->dump(out, ind+3);
      else
	    out << setw(ind+3) << "" << "/* NOOP */" << endl;
}

void PFunction::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "function ";
      if (is_auto_) out << "automatic ";

      out << pscope_name() << ";" << endl;
      if (method_of())
	    out << setw(ind) << "" << "method of " << method_of()->name << ";" << endl;

      if (return_type_)
	    return_type_->pform_dump(out, ind+8);
      else
	    out << setw(ind+8) << "" << "<implicit type>" << endl;

      dump_ports_(out, ind+2);

      dump_parameters_(out, ind+2);

      dump_localparams_(out, ind+2);

      dump_events_(out, ind+2);

      dump_wires_(out, ind+2);

      dump_var_inits_(out, ind+2);

      if (statement_)
	    statement_->dump(out, ind+2);
      else
	    out << setw(ind+2) << "" << "/* NOOP */" << endl;
}

void PLet::let_port_t::dump(ostream&out, unsigned) const
{
      if (type_) out << *type_ << " ";
      out << name_;
	// FIXME: This has not been tested and is likely wrong!
      if (range_) out << " " << range_;
      if (def_) out << "=" << *def_;
}

void PLet::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "let ";
      out << pscope_name();
      if (ports_) {
	    out << "(";
	    typedef std::list<let_port_t*>::const_iterator port_itr_t;
	    port_itr_t idx = ports_->begin();
	    (*idx)->dump(out, 0);
	    for (++idx; idx != ports_->end(); ++idx ) {
		  out << ", ";
		  (*idx)->dump(out, 0);
	    }
	    out << ")";
      }
      out << " = " << *expr_ << ";" << endl;
}

void PRelease::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "release " << *lval_ << "; /* "
	  << get_fileline() << " */" << endl;
}

void PRepeat::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "repeat (" << *expr_ << ")" << endl;
      if (statement_)
	    statement_->dump(out, ind+3);
      else
	    out << setw(ind+3) << "" << "/* NOOP */" << endl;
}

void PReturn::dump(ostream&fd, unsigned ind) const
{
      fd << setw(ind) << "" << "return (";
      if (expr_) fd << *expr_;
      fd << ")" << endl;
}

void PTask::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "task ";
      if (is_auto_) out << "automatic ";
      out << pscope_name() << ";" << endl;
      if (method_of())
	    out << setw(ind) << "" << "method of " << method_of()->name << ";" << endl;
      dump_ports_(out, ind+2);

      dump_parameters_(out, ind+2);

      dump_localparams_(out, ind+2);

      dump_events_(out, ind+2);

      dump_wires_(out, ind+2);

      dump_var_inits_(out, ind+2);

      if (statement_)
	    statement_->dump(out, ind+2);
      else
	    out << setw(ind+2) << "" << "/* NOOP */" << endl;
}

void PTaskFunc::dump_ports_(std::ostream&out, unsigned ind) const
{
      if (ports_ == 0)
	    return;

      for (unsigned idx = 0 ; idx < ports_->size() ; idx += 1) {
	    if (ports_->at(idx).port == 0) {
		  out << setw(ind) << "" << "ERROR PORT" << endl;
		  continue;
	    }

	    out << setw(ind) << "";
	    switch (ports_->at(idx).port->get_port_type()) {
		case NetNet::PINPUT:
		  out << "input ";
		  break;
		case NetNet::POUTPUT:
		  out << "output ";
		  break;
		case NetNet::PINOUT:
		  out << "inout ";
		  break;
		case NetNet::PIMPLICIT:
		  out << "PIMPLICIT";
		  break;
		case NetNet::NOT_A_PORT:
		  out << "NOT_A_PORT";
		  break;
		default:
		  assert(0);
		  break;
	    }
	    out << ports_->at(idx).port->basename();
	    if (ports_->at(idx).defe) {
		  out << " = " << *ports_->at(idx).defe;
	    }
	    out << ";" << endl;
      }
}

void PTrigger::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "-> " << event_ << ";" << endl;
}

void PNBTrigger::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "->> ";
      if (dly_) out << "#" << *dly_ << " ";
      out << event_ << ";" << endl;
}

void PWhile::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "while (" << *cond_ << ")" << endl;
      if (statement_)
	    statement_->dump(out, ind+3);
      else
	    out << setw(ind+3) << "" << "/* NOOP */" << endl;
}

void PProcess::dump(ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << type_
	  << " /* " << get_fileline() << " */" << endl;

      dump_attributes_map(out, attributes, ind+2);

      if (statement_)
	    statement_->dump(out, ind+2);
      else
	    out << setw(ind+2) << "" << "/* NOOP */" << endl;
}

void AProcess::dump(ostream&out, unsigned ind) const
{
      switch (type_) {
	  case IVL_PR_INITIAL:
	    out << setw(ind) << "" << "analog initial";
	    break;
	  case IVL_PR_ALWAYS:
	    out << setw(ind) << "" << "analog";
	    break;
	  case IVL_PR_ALWAYS_COMB:
	  case IVL_PR_ALWAYS_FF:
	  case IVL_PR_ALWAYS_LATCH:
	    assert(0);
	    break;
	  case IVL_PR_FINAL:
	    out << setw(ind) << "" << "analog final";
	    break;
      }

      out << " /* " << get_fileline() << " */" << endl;

      dump_attributes_map(out, attributes, ind+2);

      if (statement_)
	    statement_->dump(out, ind+2);
      else
	    out << setw(ind+2) << "" << "/* NOOP */" << endl;
}

void PSpecPath::dump(std::ostream&out, unsigned ind) const
{
      out << setw(ind) << "" << "specify path ";

      if (condition)
	    out << "if (" << *condition << ") ";

      out << "(";
      if (edge) {
	    if (edge > 0)
		  out << "posedge ";
	    else
		  out << "negedge ";
      }

      for (unsigned idx = 0 ;  idx < src.size() ;  idx += 1) {
	    if (idx > 0) out << ", ";
	    assert(src[idx]);
	    out << src[idx];
      }

      out << " ";
      if (polarity_) out << polarity_;
      if (full_flag_) out << "*> ";
      else out << "=> ";

      if (data_source_expression)
	    out << "(";

      for (unsigned idx = 0 ; idx < dst.size() ;  idx += 1) {
	    if (idx > 0) out << ", ";
	    assert(dst[idx]);
	    out << dst[idx];
      }

      if (data_source_expression)
	    out << " : " << *data_source_expression << ")";

      out << ") = (";
      for (unsigned idx = 0 ;  idx < delays.size() ;  idx += 1) {
	    if (idx > 0) out << ", ";
	    assert(delays[idx]);
	    out << *delays[idx];
      }
      out << ");" << endl;
}

void PGenerate::dump(ostream&out, unsigned indent) const
{
      out << setw(indent) << "" << "generate(" << id_number << ")";

      PGenerate*parent = dynamic_cast<PGenerate*>(parent_scope());
      switch (scheme_type) {
	  case GS_NONE:
	    break;
	  case GS_LOOP:
	    out << " for ("
		<< loop_index
		<< "=" << *loop_init
		<< "; " << *loop_test
		<< "; " << loop_index
		<< "=" << *loop_step << ")";
	    break;
	  case GS_CONDIT:
	    out << " if (" << *loop_test << ")";
	    break;
	  case GS_ELSE:
	    out << " else !(" << *loop_test << ")";
	    break;
	  case GS_CASE:
	    out << " case (" << *loop_test << ")";
	    break;
	  case GS_CASE_ITEM:
            assert(parent);
	    if (loop_test)
		  out << " (" << *loop_test << ") == (" << *parent->loop_test << ")";
	    else
		  out << " default:";
	    break;
	  case GS_NBLOCK:
	    out << " begin";
      }

      if (scope_name)
	    out << " : " << scope_name;

      out << endl;

      dump_localparams_(out, indent+2);

      typedef list<PGenerate::named_expr_t>::const_iterator parm_hiter_t;
      for (parm_hiter_t cur = defparms.begin()
		 ; cur != defparms.end() ;  ++ cur ) {
	    out << setw(indent+2) << "" << "defparam " << (*cur).first << " = ";
	    if ((*cur).second)
		  out << *(*cur).second << ";" << endl;
	    else
		  out << "/* ERROR */;" << endl;
      }

      dump_events_(out, indent+2);

      dump_wires_(out, indent+2);

      for (list<PGate*>::const_iterator idx = gates.begin()
		 ; idx != gates.end() ; ++ idx ) {
	    (*idx)->dump(out, indent+2);
      }

      dump_var_inits_(out, indent+2);

      for (list<PProcess*>::const_iterator idx = behaviors.begin()
		 ; idx != behaviors.end() ; ++ idx ) {
	    (*idx)->dump(out, indent+2);
      }

      for (list<AProcess*>::const_iterator idx = analog_behaviors.begin()
		 ; idx != analog_behaviors.end() ; ++ idx ) {
	    (*idx)->dump(out, indent+2);
      }

      for (list<PCallTask*>::const_iterator idx = elab_tasks.begin()
		 ; idx != elab_tasks.end() ; ++ idx ) {
	    (*idx)->dump(out, indent+2);
      }

      typedef map<perm_string,LineInfo*>::const_iterator genvar_iter_t;
      for (genvar_iter_t cur = genvars.begin()
		 ; cur != genvars.end() ; ++ cur ) {
	    out << setw(indent+2) << "" << "genvar " << ((*cur).first) << ";" << endl;
      }

      for (list<PGenerate*>::const_iterator idx = generate_schemes.begin()
		 ; idx != generate_schemes.end() ; ++ idx ) {
	    (*idx)->dump(out, indent+2);
      }

      if (scheme_type == GS_NBLOCK) {
	    out << setw(indent) << "" << "end endgenerate" << endl;
      } else {
	    out << setw(indent) << "" << "endgenerate" << endl;
      }
}

void LexicalScope::dump_typedefs_(ostream&out, unsigned indent) const
{
      typedef map<perm_string,data_type_t*>::const_iterator iter_t;
      for (iter_t cur = typedefs.begin() ; cur != typedefs.end() ; ++ cur) {
	    out << setw(indent) << "" << "typedef of " << cur->first << ":" << endl;
	    cur->second->pform_dump(out, indent+4);
      }
}

void LexicalScope::dump_parameters_(ostream&out, unsigned indent) const
{
      typedef map<perm_string,param_expr_t*>::const_iterator parm_iter_t;
      for (parm_iter_t cur = parameters.begin()
		 ; cur != parameters.end() ; ++ cur ) {
	    out << setw(indent) << "" << "parameter ";
	    if (cur->second->data_type)
	          cur->second->data_type->debug_dump(out);
	    else
		  out << "(nil type)";
	    out << " " << (*cur).first << " = ";
	    if ((*cur).second->expr)
		  out << *(*cur).second->expr;
	    else
		  out << "/* ERROR */";
	    for (LexicalScope::range_t*tmp = (*cur).second->range
		       ; tmp ; tmp = tmp->next) {
		  if (tmp->exclude_flag)
			out << " exclude ";
		  else
			out << " from ";
		  if (tmp->low_open_flag)
			out << "(";
		  else
			out << "[";
		  if (tmp->low_expr)
			out << *(tmp->low_expr);
		  else if (tmp->low_open_flag==false)
			out << "-inf";
		  else
			out << "<nil>";
		  out << ":";
		  if (tmp->high_expr)
			out << *(tmp->high_expr);
		  else if (tmp->high_open_flag==false)
			out << "inf";
		  else
			out << "<nil>";
		  if (tmp->high_open_flag)
			out << ")";
		  else
			out << "]";
	    }
	    out << ";" << endl;
      }
}

void LexicalScope::dump_localparams_(ostream&out, unsigned indent) const
{
      typedef map<perm_string,param_expr_t*>::const_iterator parm_iter_t;
      for (parm_iter_t cur = localparams.begin()
		 ; cur != localparams.end() ; ++ cur ) {
	    out << setw(indent) << "" << "localparam ";
	    if (cur->second->data_type) {
		  cur->second->data_type->debug_dump(out);
		  out << " ";
	    }
	    out << (*cur).first << " = ";
	    if ((*cur).second->expr)
		  out << *(*cur).second->expr << ";" << endl;
	    else
		  out << "/* ERROR */;" << endl;
      }
}

void LexicalScope::dump_enumerations_(ostream&out, unsigned indent) const
{
      for (set<enum_type_t*>::const_iterator cur = enum_sets.begin()
		 ; cur != enum_sets.end() ; ++ cur) {
	    out << setw(indent) << "" << "enum {" << endl;

	    for (list<named_pexpr_t>::const_iterator idx = (*cur)->names->begin()
		       ; idx != (*cur)->names->end() ; ++ idx) {
		  out << setw(indent+4) << "" << idx->name
		      << " = " << idx->parm << endl;
	    }

	    out << setw(indent) << "" << "}" << endl;
      }
}

void LexicalScope::dump_events_(ostream&out, unsigned indent) const
{
      for (map<perm_string,PEvent*>::const_iterator cur = events.begin()
		 ; cur != events.end() ; ++ cur ) {
	    PEvent*ev = (*cur).second;
	    out << setw(indent) << "" << "event " << ev->name() << "; // "
		<< ev->get_fileline() << endl;
      }
}

void LexicalScope::dump_wires_(ostream&out, unsigned indent) const
{
	// Iterate through and display all the wires.
      for (map<perm_string,PWire*>::const_iterator wire = wires.begin()
		 ; wire != wires.end() ; ++ wire ) {

	    (*wire).second->dump(out, indent);
      }
}

void LexicalScope::dump_var_inits_(ostream&out, unsigned indent) const
{
	// Iterate through and display all the register initializations.
      for (unsigned idx = 0; idx < var_inits.size(); idx += 1) {
	    var_inits[idx]->dump(out, indent);
      }
}

void PScopeExtra::dump_classes_(ostream&out, unsigned indent) const
{
	// Dump the task definitions.
      typedef map<perm_string,PClass*>::const_iterator class_iter_t;
      for (class_iter_t cur = classes.begin()
		 ; cur != classes.end() ; ++ cur ) {
	    cur->second->dump(out, indent);
      }
}

void PScopeExtra::dump_tasks_(ostream&out, unsigned indent) const
{
	// Dump the task definitions.
      typedef map<perm_string,PTask*>::const_iterator task_iter_t;
      for (task_iter_t cur = tasks.begin()
		 ; cur != tasks.end() ; ++ cur ) {
	    out << setw(indent) << "" << "task " << (*cur).first << ";" << endl;
	    (*cur).second->dump(out, indent+2);
	    out << setw(indent) << "" << "endtask;" << endl;
      }
}

void PScopeExtra::dump_funcs_(ostream&out, unsigned indent) const
{
	// Dump the task definitions.
      typedef map<perm_string,PFunction*>::const_iterator task_iter_t;
      for (task_iter_t cur = funcs.begin()
		 ; cur != funcs.end() ; ++ cur ) {
	    out << setw(indent) << "" << "function " << (*cur).first << ";" << endl;
	    (*cur).second->dump(out, indent+2);
	    out << setw(indent) << "" << "endfunction;" << endl;
      }
}

void PClass::dump(ostream&out, unsigned indent) const
{
      out << setw(indent) << "" << "class " << type->name << ";" << endl;
      type->pform_dump(out, indent+2);
      type->pform_dump_init(out, indent+2);
      dump_tasks_(out, indent+2);
      dump_funcs_(out, indent+2);
      out << setw(indent) << "" << "endclass" << endl;
}

void Module::dump_specparams_(ostream&out, unsigned indent) const
{
      typedef map<perm_string,param_expr_t*>::const_iterator parm_iter_t;
      for (parm_iter_t cur = specparams.begin()
		 ; cur != specparams.end() ; ++ cur ) {
	    out << setw(indent) << "" << "specparam ";
	    if (cur->second->data_type)
		  cur->second->data_type->debug_dump(out);
	    else
		  out << "(nil type)";

	    out << (*cur).first << " = ";
	    if ((*cur).second->expr)
		  out << *(*cur).second->expr << ";" << endl;
	    else
		  out << "/* ERROR */;" << endl;
      }
}

void Module::dump(ostream&out) const
{
      if (attributes.begin() != attributes.end()) {
	    out << "(* ";
	    for (map<perm_string,PExpr*>::const_iterator idx = attributes.begin()
		 ; idx != attributes.end() ; ++ idx ) {
		    if (idx != attributes.begin()) {
			out << " , ";
		    }
		    out << (*idx).first;
		    if ((*idx).second) {
			out << " = " << *(*idx).second;
		    }
	    }
	    out << " *)  ";
      }

      out << "module " << mod_name() << ";";
      if (is_cell) out << "  // Is in `celldefine.";
      out << endl;

      for (unsigned idx = 0 ;  idx < ports.size() ;  idx += 1) {
	    port_t*cur = ports[idx];

	    if (cur == 0) {
		  out << "    unconnected" << endl;
		  continue;
	    }

	    out << "    ." << cur->name << "(" << *cur->expr[0];
	    for (unsigned wdx = 1 ;  wdx < cur->expr.size() ;  wdx += 1) {
		  out << ", " << *cur->expr[wdx];
	    }

	    out << ")" << endl;
      }

      for (map<perm_string,Module*>::const_iterator cur = nested_modules.begin()
		 ; cur != nested_modules.end() ; ++cur) {
	    out << setw(4) << "" << "Nested module " << cur->first << ";" << endl;
      }

      dump_typedefs_(out, 4);

      dump_parameters_(out, 4);

      dump_localparams_(out, 4);

      dump_specparams_(out, 4);

      dump_enumerations_(out, 4);

      dump_classes_(out, 4);

      typedef map<perm_string,LineInfo*>::const_iterator genvar_iter_t;
      for (genvar_iter_t cur = genvars.begin()
		 ; cur != genvars.end() ; ++ cur ) {
	    out << "    genvar " << ((*cur).first) << ";" << endl;
      }

      typedef list<PGenerate*>::const_iterator genscheme_iter_t;
      for (genscheme_iter_t cur = generate_schemes.begin()
		 ; cur != generate_schemes.end() ; ++ cur ) {
	    (*cur)->dump(out, 4);
      }

      typedef list<Module::named_expr_t>::const_iterator parm_hiter_t;
      for (parm_hiter_t cur = defparms.begin()
		 ; cur != defparms.end() ;  ++ cur ) {
	    out << "    defparam " << (*cur).first << " = ";
	    if ((*cur).second)
		  out << *(*cur).second << ";" << endl;
	    else
		  out << "/* ERROR */;" << endl;
      }

      dump_events_(out, 4);

	// Iterate through and display all the wires.
      dump_wires_(out, 4);

	// Dump the task definitions.
      dump_tasks_(out, 4);

	// Dump the function definitions.
      dump_funcs_(out, 4);


	// Iterate through and display all the gates
      for (list<PGate*>::const_iterator gate = gates_.begin()
		 ; gate != gates_.end() ; ++ gate ) {

	    (*gate)->dump(out);
      }

      dump_var_inits_(out, 4);

      for (list<PProcess*>::const_iterator behav = behaviors.begin()
		 ; behav != behaviors.end() ; ++ behav ) {

	    (*behav)->dump(out, 4);
      }

      for (list<AProcess*>::const_iterator idx = analog_behaviors.begin()
		 ; idx != analog_behaviors.end() ; ++ idx) {
	    (*idx)->dump(out, 4);
      }

      for (list<PCallTask*>::const_iterator idx = elab_tasks.begin()
		 ; idx != elab_tasks.end() ; ++ idx ) {
	    (*idx)->dump(out, 4);
      }

      for (list<PSpecPath*>::const_iterator spec = specify_paths.begin()
		 ; spec != specify_paths.end() ; ++ spec ) {

	    (*spec)->dump(out, 4);
      }

      out << "endmodule" << endl;
}

void pform_dump(ostream&out, Module*mod)
{
      mod->dump(out);
}

void PUdp::dump(ostream&out) const
{
      out << "primitive " << name_ << "(" << ports[0];
      for (unsigned idx = 1 ;  idx < ports.count() ;  idx += 1)
	    out << ", " << ports[idx];
      out << ");" << endl;

      if (sequential)
	    out << "    reg " << ports[0] << ";" << endl;

      out << "    table" << endl;
      for (unsigned idx = 0 ;  idx < tinput.count() ;  idx += 1) {
	    out << "     ";
	    for (unsigned chr = 0 ;  chr < tinput[idx].length() ;  chr += 1)
		  out << " " << tinput[idx][chr];

	    if (sequential)
		  out << " : " << tcurrent[idx];

	    out << " : " << toutput[idx] << " ;" << endl;
      }
      out << "    endtable" << endl;

      if (sequential)
	    out << "    initial " << ports[0] << " = 1'b" << initial
		<< ";" << endl;

	// Dump the attributes for the primitive as attribute
	// statements.
      for (map<string,PExpr*>::const_iterator idx = attributes.begin()
		 ; idx != attributes.end() ; ++ idx ) {
	    out << "    attribute " << (*idx).first;
	    if ((*idx).second)
		  out << " = " << *(*idx).second;
	    out << endl;
      }

      out << "endprimitive" << endl;
}

void pform_dump(std::ostream&out, const ivl_nature_s*nat)
{
      out << "nature " << nat->name() << endl;
      out << "    access " << nat->access() << ";" << endl;
      out << "endnature" << endl;
}

void pform_dump(std::ostream&out, const ivl_discipline_s*dis)
{
      out << "discipline " << dis->name() << endl;
      out << "    domain " << dis->domain() << ";" << endl;
      if (const ivl_nature_s*tmp = dis->potential())
	    out << "    potential " << tmp->name() << ";" << endl;
      if (const ivl_nature_s*tmp = dis->flow())
	    out << "    flow " << tmp->name() << ";" << endl;
      out << "enddiscipline" << endl;
}

void pform_dump(std::ostream&fd, const PClass*cl)
{
      cl->dump(fd, 0);
}

void pform_dump(std::ostream&out, const PPackage*pac)
{
      pac->pform_dump(out);
}

void PPackage::pform_dump(std::ostream&out) const
{
      out << "package " << pscope_name() << endl;
      dump_localparams_(out, 4);
      dump_parameters_(out, 4);
      dump_typedefs_(out, 4);
      dump_enumerations_(out, 4);
      dump_wires_(out, 4);
      dump_tasks_(out, 4);
      dump_funcs_(out, 4);
      dump_var_inits_(out, 4);
      out << "endpackage" << endl;
}

void pform_dump(std::ostream&fd, const PTaskFunc*obj)
{
      obj->dump(fd, 0);
}
