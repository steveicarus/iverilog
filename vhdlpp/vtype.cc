/*
 * Copyright (c) 2011 Stephen Williams (steve@icarus.com)
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

# include  "vtype.h"
# include  "parse_types.h"
# include  "compiler.h"
# include  <map>
# include  <typeinfo>
# include  <cassert>
# include  <algorithm>

using namespace std;

VType::~VType()
{
}

void VType::show(ostream&out) const
{
      write_to_stream(out);
}

perm_string VType::get_generic_typename() const
{
    char buf[16] = {0,};
    snprintf(buf, 16, "type_%p", this);
    return lex_strings.make(buf);
}

VTypePrimitive::VTypePrimitive(VTypePrimitive::type_t tt, bool packed)
: type_(tt), packed_(packed)
{
}

VTypePrimitive::~VTypePrimitive()
{
}

void VTypePrimitive::show(ostream&out) const
{
      switch (type_) {
	  case BIT:
	    out << "BIT";
	    break;
	  case INTEGER:
	    out << "INTEGER";
	    break;
	  case NATURAL:
	    out << "NATURAL";
	    break;
	  case REAL:
	    out << "REAL";
	    break;
	  case STDLOGIC:
	    out << "STD_LOGIC";
	    break;
	  case TIME:
	    out << "TIME";
	    break;
      }
}

int VTypePrimitive::get_width(ScopeBase*) const
{
    switch(type_) {
        case BIT:
        case STDLOGIC:
            return 1;

        case INTEGER:
        case NATURAL:
            return 32;

        default:
            std::cerr << "sorry: primitive type " << type_ <<
                " has no get_width() implementation." << std::endl;
            break;
    }

    return -1;
}

VTypeArray::range_t*VTypeArray::range_t::clone() const
{
    return new VTypeArray::range_t(safe_clone(msb_), safe_clone(lsb_), direction_);
}

VTypeArray::VTypeArray(const VType*element, const vector<VTypeArray::range_t>&r, bool sv)
: etype_(element), ranges_(r), signed_flag_(sv), parent_(NULL)
{
}

/*
 * Create a VTypeArray range set from a list of parsed ranges.
 * FIXME: We are copying pointers from the ExpRange object into the
 * range_t. This means that we cannot delete the ExpRange object
 * unless we invent a way to remove the pointers from that object. So
 * this is a memory leak. Something to fix.
 */
VTypeArray::VTypeArray(const VType*element, std::list<ExpRange*>*r, bool sv)
: etype_(element), ranges_(r->size()), signed_flag_(sv), parent_(NULL)
{
      for (size_t idx = 0 ; idx < ranges_.size() ; idx += 1) {
	    ExpRange*curp = r->front();
	    r->pop_front();
	    ranges_[idx] = range_t(curp->msb(), curp->lsb(),
		(curp->direction() == ExpRange::DOWNTO
			? true
			: false));
      }
}

VTypeArray::VTypeArray(const VType*element, int msb, int lsb, bool sv)
: etype_(element), ranges_(1), signed_flag_(sv), parent_(NULL)
{
      bool down_to = msb > lsb;
      ranges_[0] = range_t(new ExpInteger(msb), new ExpInteger(lsb), down_to);
}

VTypeArray::~VTypeArray()
{
}

VType*VTypeArray::clone() const {
    std::vector<range_t> new_ranges;
    new_ranges.reserve(ranges_.size());
    for(std::vector<range_t>::const_iterator it = ranges_.begin();
            it != ranges_.end(); ++it) {
        new_ranges.push_back(*(it->clone()));
    }
    VTypeArray*a = new VTypeArray(etype_->clone(), new_ranges, signed_flag_);
    a->set_parent_type(parent_);
    return a;
}

const VType* VTypeArray::basic_type(bool typedef_allowed) const
{
    const VType*t = etype_;
    const VTypeDef*tdef = NULL;
    bool progress = false;

    do {
        progress = false;

        if((tdef = dynamic_cast<const VTypeDef*>(t))) {
            t = tdef->peek_definition();
        }

        if(const VTypeArray*arr = dynamic_cast<const VTypeArray*>(t)) {
            t = arr->element_type();
            progress = true;
        } else if(tdef) { // return the typedef if it does not define an array
            t = typedef_allowed ? tdef : tdef->peek_definition();
        }
    } while(progress);

    return t;
}

void VTypeArray::show(ostream&out) const
{
      out << "array ";
      for (vector<range_t>::const_iterator cur = ranges_.begin()
		 ; cur != ranges_.end() ; ++cur) {
	    out << "(";
	    if (cur->msb())
		  cur->msb()->write_to_stream(out);
	    else
		  out << "<>";
	    out << " downto ";
	    if (cur->lsb())
		  cur->lsb()->write_to_stream(out);
	    else
		  out << "<>";
	    out << ")";
      }
      out << " of ";
      if (signed_flag_)
	    out << "signed ";
      if (etype_)
	    etype_->show(out);
      else
	    out << "<nil>";
}

int VTypeArray::get_width(ScopeBase*scope) const
{
      int64_t size = 1;

      for(vector<range_t>::const_iterator it = ranges_.begin();
              it != ranges_.end(); ++it) {
          const VTypeArray::range_t&dim = *it;
          int64_t msb_val, lsb_val;

          if(dim.is_box())
              return -1;

          if(!dim.msb()->evaluate(scope, msb_val))
              return -1;

          if(!dim.lsb()->evaluate(scope, lsb_val))
              return -1;

          size *= 1 + labs(msb_val - lsb_val);
      }

      return element_type()->get_width(scope) * size;
}

bool VTypeArray::is_unbounded() const {
    for(std::vector<range_t>::const_iterator it = ranges_.begin();
            it != ranges_.end(); ++it)
    {
        if(it->is_box())
            return true;
    }

    return etype_->is_unbounded();
}

bool VTypeArray::is_variable_length(ScopeBase*scope) const {
    int64_t dummy;

    if(is_unbounded())
        return true;

    for(std::vector<range_t>::const_iterator it = ranges_.begin();
            it != ranges_.end(); ++it)
    {
        if(!it->lsb()->evaluate(scope, dummy))
            return true;

        if(!it->msb()->evaluate(scope, dummy))
            return true;
    }

    return etype_->is_variable_length(scope);
}

void VTypeArray::evaluate_ranges(ScopeBase*scope) {
    for(std::vector<range_t>::iterator it = ranges_.begin(); it != ranges_.end(); ++it ) {
        int64_t lsb_val = -1, msb_val = -1;

        if(it->msb()->evaluate(scope, msb_val) && it->lsb()->evaluate(scope, lsb_val)) {
            assert(lsb_val >= 0);
            assert(msb_val >= 0);
            *it = range_t(new ExpInteger(msb_val), new ExpInteger(lsb_val), msb_val > lsb_val);
        }
    }
}

VTypeRange::VTypeRange(const VType*base)
: base_(base)
{
}

VTypeRange::~VTypeRange()
{
}

VTypeRangeConst::VTypeRangeConst(const VType*base, int64_t start_val, int64_t end_val)
: VTypeRange(base), start_(start_val), end_(end_val)
{
}

VTypeRangeExpr::VTypeRangeExpr(const VType*base, Expression*start_expr,
                               Expression*end_expr, bool downto)
: VTypeRange(base), start_(start_expr), end_(end_expr), downto_(downto)
{
}

VTypeRangeExpr::~VTypeRangeExpr()
{
    delete start_;
    delete end_;
}

VType*VTypeRangeExpr::clone() const {
    return new VTypeRangeExpr(base_type()->clone(), start_->clone(),
                              end_->clone(), downto_);
}

VTypeEnum::VTypeEnum(const std::list<perm_string>*names)
: names_(names->size())
{
      size_t idx = 0;

      for (list<perm_string>::const_iterator cur = names->begin()
		 ; cur != names->end() ; ++cur, ++idx) {
	    names_[idx] = *cur;
      }
}

VTypeEnum::~VTypeEnum()
{
}

void VTypeEnum::show(ostream&out) const
{
      out << "(";
      if (names_.size() >= 1)
	    out << names_[0];
      for (size_t idx = 1 ; idx < names_.size() ; idx += 1)
	    out << ", " << names_[idx];
      out << ")";
}

bool VTypeEnum::has_name(perm_string name) const
{
      return std::find(names_.begin(), names_.end(), name) != names_.end();
}

VTypeRecord::VTypeRecord(std::list<element_t*>*elements)
: elements_(elements->size())
{
      for (size_t idx = 0 ; idx < elements_.size() ; idx += 1) {
	    elements_[idx] = elements->front();
	    elements->pop_front();
      }
      delete elements;
}

VTypeRecord::~VTypeRecord()
{
      for (size_t idx = 0 ; idx < elements_.size() ; idx += 1)
	    delete elements_[idx];
}

void VTypeRecord::show(ostream&out) const
{
      write_to_stream(out);
}

int VTypeRecord::get_width(ScopeBase*scope) const
{
    int width = 0;

    for(vector<element_t*>::const_iterator it = elements_.begin();
            it != elements_.end(); ++it) {
        int w = (*it)->peek_type()->get_width(scope);

        if(w < 0)
            return -1;

        width += w;
    }

    return width;
}

const VTypeRecord::element_t* VTypeRecord::element_by_name(perm_string name, int*index) const
{
      for (vector<element_t*>::const_iterator cur = elements_.begin()
		 ; cur != elements_.end() ; ++cur) {
	    element_t*curp = *cur;
	    if (curp->peek_name() == name) {
		  if (index)
		      *index = std::distance(elements_.begin(), cur);

		  return curp;
	    }
      }

      return 0;
}

VTypeRecord::element_t::element_t(perm_string name, const VType*typ)
: name_(name), type_(typ)
{
}

VTypeDef::VTypeDef(perm_string nam)
: name_(nam), type_(0)
{
}

VTypeDef::VTypeDef(perm_string nam, const VType*typ)
: name_(nam), type_(typ)
{
}

VTypeDef::~VTypeDef()
{
}

void VTypeDef::set_definition(const VType*typ)
{
      assert(type_ == 0);
      type_ = typ;
}
