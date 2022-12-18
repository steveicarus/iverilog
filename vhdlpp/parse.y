
%define api.pure
%lex-param { yyscan_t yyscanner }
%parse-param {yyscan_t yyscanner  }
%parse-param {const char*file_path}
%parse-param {perm_string parse_library_name}
%{
/*
 * Copyright (c) 2011-2021 Stephen Williams (steve@icarus.com)
 * Copyright CERN 2012-2016 / Stephen Williams (steve@icarus.com),
 * @author Maciej Suminski (maciej.suminski@cern.ch)
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

# include "vhdlpp_config.h"
# include "vhdlint.h"
# include "vhdlreal.h"
# include "compiler.h"
# include "parse_api.h"
# include "parse_misc.h"
# include "architec.h"
# include "expression.h"
# include "sequential.h"
# include "subprogram.h"
# include "package.h"
# include "vsignal.h"
# include "vtype.h"
# include "std_funcs.h"
# include "std_types.h"
# include  <cstdarg>
# include  <cstring>
# include  <list>
# include  <stack>
# include  <map>
# include  <vector>
# include  "parse_types.h"
# include  <ivl_assert.h>
# include  <assert.h>

using namespace std;

inline void FILE_NAME(LineInfo*tmp, const struct yyltype&where)
{
      tmp->set_lineno(where.first_line);
      tmp->set_file(filename_strings.make(where.text));
}


/* Recent version of bison expect that the user supply a
   YYLLOC_DEFAULT macro that makes up a yylloc value from existing
   values. I need to supply an explicit version to account for the
   text field, that otherwise won't be copied. */
# define YYLLOC_DEFAULT(Current, Rhs, N)  do {       \
  (Current).first_line   = (Rhs)[1].first_line;      \
  (Current).text         = file_path; /*(Rhs)[1].text;*/   } while (0)

static void yyerror(YYLTYPE*yyllocp,yyscan_t yyscanner,const char*file_path,bool, const char*msg);

int parse_errors = 0;
int parse_sorrys = 0;

/*
 * The parser calls yylex to get the next lexical token. It is only
 * called by the bison-generated parser.
 */
extern int yylex(union YYSTYPE*yylvalp,YYLTYPE*yyllocp,yyscan_t yyscanner);


/*
 * Create an initial scope that collects all the global
 * declarations. Also save a stack of previous scopes, as a way to
 * manage lexical scopes.
 */
static ActiveScope*active_scope = new ActiveScope;
static stack<ActiveScope*> scope_stack;
static SubprogramHeader*active_sub = NULL;
static ActiveScope*arc_scope = NULL;

/*
 * When a scope boundary starts, call the push_scope function to push
 * a scope context. Preload this scope context with the contents of
 * the parent scope, then make this the current scope. When the scope
 * is done, the pop_scope function pops the scope off the stack and
 * resumes the scope that was the parent.
 */
static void push_scope(void)
{
      assert(active_scope);
      scope_stack.push(active_scope);
      active_scope = new ActiveScope (active_scope);
}

static void pop_scope(void)
{
      delete active_scope;
      assert(! scope_stack.empty());
      active_scope = scope_stack.top();
      scope_stack.pop();
}

static bool is_subprogram_param(perm_string name)
{
    if(!active_sub)
        return false;

    return (active_sub->find_param(name) != NULL);
}

void preload_global_types(void)
{
      generate_global_types(active_scope);
}

//Remove the scope created at the beginning of parser's work.
//After the parsing active_scope should keep it's address

static void delete_global_scope(void)
{
    active_scope->destroy_global_scope();
    delete active_scope;
}

//delete global entities that were gathered over the parsing process
static void delete_design_entities(void)
{
      for(map<perm_string,Entity*>::iterator cur = design_entities.begin()
      ; cur != design_entities.end(); ++cur)
        delete cur->second;
}

//clean the mess caused by the parser
void parser_cleanup(void)
{
    delete_design_entities();
    delete_global_scope();
    delete_std_funcs();
    lex_strings.cleanup();
}

const VType*parse_type_by_name(perm_string name)
{
      return active_scope->find_type(name);
}

// This function is called when an aggregate expression is detected by
// the parser. It makes the ExpAggregate. It also tries to detect the
// special case that the aggregate is really a primary. The problem is
// that this:
//   ( <expression> )
// also matches the pattern:
//   ( [ choices => ] <expression> ... )
// so try to assume that a single expression in parentheses is a
// primary and fix the parse by returning an Expression instead of an
// ExpAggregate.
static Expression*aggregate_or_primary(const YYLTYPE&loc, std::list<ExpAggregate::element_t*>*el)
{
      if (el->size() != 1) {
	    ExpAggregate*tmp = new ExpAggregate(el);
	    FILE_NAME(tmp,loc);
	    return tmp;
      }

      ExpAggregate::element_t*el1 = el->front();
      if (el1->count_choices() > 0) {
	    ExpAggregate*tmp = new ExpAggregate(el);
	    FILE_NAME(tmp,loc);
	    return tmp;
      }

      return el1->extract_expression();
}

static list<VTypeRecord::element_t*>* record_elements(list<perm_string>*names,
						      const VType*type)
{
      list<VTypeRecord::element_t*>*res = new list<VTypeRecord::element_t*>;

      for (list<perm_string>::iterator cur = names->begin()
		 ; cur != names->end() ; ++cur) {
	    res->push_back(new VTypeRecord::element_t(*cur, type));
      }

      return res;
}

static void touchup_interface_for_functions(std::list<InterfacePort*>*ports)
{
      for (list<InterfacePort*>::iterator cur = ports->begin()
		 ; cur != ports->end() ; ++cur) {
	    InterfacePort*curp = *cur;
	    if (curp->mode == PORT_NONE)
		  curp->mode = PORT_IN;
      }
}

%}


%union {
      port_mode_t port_mode;

      char*text;

      std::list<perm_string>* name_list;

      bool flag;
      int64_t uni_integer;
      double  uni_real;

      Expression*expr;
      std::list<Expression*>* expr_list;

      SequentialStmt* sequ;
      std::list<SequentialStmt*>*sequ_list;

      IfSequential::Elsif*elsif;
      std::list<IfSequential::Elsif*>*elsif_list;

      ExpConditional::case_t*exp_options;
      std::list<ExpConditional::case_t*>*exp_options_list;

      CaseSeqStmt::CaseStmtAlternative* case_alt;
      std::list<CaseSeqStmt::CaseStmtAlternative*>* case_alt_list;

      named_expr_t*named_expr;
      std::list<named_expr_t*>*named_expr_list;
      entity_aspect_t* entity_aspect;
      instant_list_t* instantiation_list;
      std::pair<instant_list_t*, ExpName*>* component_specification;

      const VType* vtype;

      ExpRange*range;
      std::list<ExpRange*>*range_list;
      ExpRange::range_dir_t range_dir;

      ExpArithmetic::fun_t arithmetic_op;
      std::list<struct adding_term>*adding_terms;

      ExpAggregate::choice_t*choice;
      std::list<ExpAggregate::choice_t*>*choice_list;
      ExpAggregate::element_t*element;
      std::list<ExpAggregate::element_t*>*element_list;

      std::list<VTypeRecord::element_t*>*record_elements;

      std::list<InterfacePort*>* interface_list;

      Architecture::Statement* arch_statement;
      std::list<Architecture::Statement*>* arch_statement_list;

      ReportStmt::severity_t severity;

      SubprogramHeader*subprogram;

      file_open_info_t*file_info;
};

  /* The keywords are all tokens. */
%token K_abs K_access K_after K_alias K_all K_and K_architecture
%token K_array K_assert K_assume K_assume_guarantee K_attribute
%token K_begin K_block K_body K_buffer K_bus
%token K_case K_component K_configuration K_constant K_context K_cover
%token K_default K_disconnect K_downto
%token K_else K_elsif K_end K_entity K_exit
%token K_fairness K_file K_for K_force K_function
%token K_generate K_generic K_group K_guarded
%token K_if K_impure K_in K_inertial K_inout K_is
%token K_label K_library K_linkage K_literal K_loop
%token K_map K_mod
%token K_nand K_new K_next K_nor K_not K_null
%token K_of K_on K_open K_or K_others K_out
%token K_package K_parameter K_port K_postponed K_procedure K_process
%token K_property K_protected K_pure
%token K_range K_record K_register K_reject K_release K_rem K_report
%token K_restrict K_restrict_guarantee K_return K_reverse_range K_rol K_ror
%token K_select K_sequence K_severity K_signal K_shared
%token K_sla K_sll K_sra K_srl K_strong K_subtype
%token K_then K_to K_transport K_type
%token K_unaffected K_units K_until K_use
%token K_variable K_vmode K_vprop K_vunit
%token K_wait K_when K_while K_with
%token K_xnor K_xor
 /* Identifiers that are not keywords are identifiers. */
%token <text> IDENTIFIER
%token <uni_integer> INT_LITERAL
%token <uni_real> REAL_LITERAL
%token <text> STRING_LITERAL CHARACTER_LITERAL BITSTRING_LITERAL
 /* compound symbols */
%token LEQ GEQ VASSIGN NE BOX EXP ARROW DLT DGT CC M_EQ M_NE M_LT M_LEQ M_GT M_GEQ

 /* The rules may have types. */

%type <arithmetic_op> adding_operator
%type <adding_terms> simple_expression_terms

%type <interface_list> interface_element interface_list
%type <interface_list> port_clause port_clause_opt
%type <interface_list> generic_clause generic_clause_opt
%type <interface_list> parameter_list parameter_list_opt
%type <port_mode>  mode mode_opt

%type <entity_aspect> entity_aspect entity_aspect_opt binding_indication binding_indication_semicolon_opt
%type <instantiation_list> instantiation_list
%type <component_specification> component_specification

%type <arch_statement> concurrent_statement component_instantiation_statement
%type <arch_statement> concurrent_conditional_signal_assignment
%type <arch_statement> concurrent_signal_assignment_statement concurrent_simple_signal_assignment
%type <arch_statement> concurrent_assertion_statement
%type <arch_statement> for_generate_statement generate_statement if_generate_statement
%type <arch_statement> process_statement selected_signal_assignment
%type <arch_statement_list> architecture_statement_part generate_statement_body

%type <choice> choice
%type <choice_list> choices
%type <element> element_association
%type <element_list> element_association_list

%type <expr> expression factor primary relation
%type <expr> expression_logical expression_logical_and expression_logical_or
%type <expr> expression_logical_xnor expression_logical_xor
%type <expr> name prefix selected_name indexed_name
%type <expr> shift_expression signal_declaration_assign_opt
%type <expr> simple_expression simple_expression_2 term
%type <expr> variable_declaration_assign_opt waveform_element interface_element_expression

%type <expr_list> waveform waveform_elements
%type <expr_list> name_list expression_list argument_list argument_list_opt
%type <expr_list> process_sensitivity_list process_sensitivity_list_opt
%type <expr_list> selected_names use_clause

%type <file_info> file_open_information file_open_information_opt

%type <named_expr> association_element
%type <named_expr_list> association_list port_map_aspect port_map_aspect_opt
%type <named_expr_list> generic_map_aspect generic_map_aspect_opt

%type <vtype> composite_type_definition record_type_definition
%type <vtype> subtype_indication type_definition

%type <record_elements> element_declaration element_declaration_list

%type <text> architecture_body_start package_declaration_start
%type <text> package_body_start process_start
%type <text> identifier_opt identifier_colon_opt logical_name suffix instantiated_unit

%type <name_list> logical_name_list identifier_list
%type <name_list> enumeration_literal_list enumeration_literal

%type <sequ_list> if_statement_else list_of_statements
%type <sequ_list> sequence_of_statements subprogram_statement_part
%type <sequ> sequential_statement if_statement signal_assignment signal_assignment_statement
%type <sequ> case_statement procedure_call procedure_call_statement
%type <sequ> loop_statement variable_assignment variable_assignment_statement
%type <sequ> assertion_statement report_statement return_statement wait_statement

%type <range> range
%type <range_list> range_list index_constraint
%type <range_dir> direction

%type <case_alt> case_statement_alternative
%type <case_alt_list> case_statement_alternative_list

%type <elsif> if_statement_elsif
%type <elsif_list> if_statement_elsif_list if_statement_elsif_list_opt

%type <exp_options> else_when_waveform selected_waveform
%type <exp_options_list> else_when_waveforms else_when_waveforms_opt selected_waveform_list

%type <subprogram> function_specification procedure_specification
%type <subprogram> subprogram_specification subprogram_body_start

%type <severity> severity severity_opt

%%

 /* The design_file is the root for the VHDL parse. This rule is also
    where I put some run-time initializations. */
design_file : { yylloc.text = file_path; } design_units ;

adding_operator
  : '+' { $$ = ExpArithmetic::PLUS; }
  | '-' { $$ = ExpArithmetic::MINUS; }
  | '&' { $$ = ExpArithmetic::xCONCAT; }
  ;

architecture_body
  : architecture_body_start
    K_of IDENTIFIER
      { bind_entity_to_active_scope($3, active_scope); }
    K_is block_declarative_items_opt
    K_begin architecture_statement_part K_end K_architecture_opt identifier_opt ';'
      { Architecture*tmp = new Architecture(lex_strings.make($1),
					    *active_scope, *$8);
	FILE_NAME(tmp, @1);
	bind_architecture_to_entity($3, tmp);
	if ($11 && tmp->get_name() != $11)
	      errormsg(@1, "Architecture name doesn't match closing name.\n");
	delete[]$1;
	delete[]$3;
	delete $8;
	pop_scope();
	assert(arc_scope);
	arc_scope = NULL;
	if ($11) delete[]$11;
      }
  ;

architecture_body_start
  : K_architecture IDENTIFIER
      { $$ = $2;
	push_scope();
	assert(!arc_scope);
	arc_scope = active_scope;
      }
  ;
/*
 * The architecture_statement_part is a list of concurrent
 * statements.
 */
architecture_statement_part
  : architecture_statement_part concurrent_statement
      { std::list<Architecture::Statement*>*tmp = $1;
	if ($2) tmp->push_back($2);
	$$ = tmp;
      }
  | concurrent_statement
      { std::list<Architecture::Statement*>*tmp = new std::list<Architecture::Statement*>;
	if ($1) tmp->push_back($1);
	$$ = tmp;
      }

  | error ';'
      { $$ = 0;
	errormsg(@1, "Syntax error in architecture statement.\n");
	yyerrok;
      }
  ;

argument_list : '(' expression_list ')' { $$ = $2; };

argument_list_opt
  : argument_list { $$ = $1; }
  | { $$ = 0; }
  ;

assertion_statement
  : K_assert expression report_statement
      { ReportStmt*report = dynamic_cast<ReportStmt*>($3);
        assert(report);
	AssertStmt*tmp = new AssertStmt($2, report->message(), report->severity());
        delete report;
	FILE_NAME(tmp,@2);
	$$ = tmp;
      }
  | K_assert expression severity_opt ';'
      { AssertStmt*tmp = new AssertStmt($2, NULL, $3);
	FILE_NAME(tmp,@2);
	$$ = tmp;
      }
  ;

association_element
  : IDENTIFIER ARROW expression
      { named_expr_t*tmp = new named_expr_t(lex_strings.make($1), $3);
	delete[]$1;
	$$ = tmp;
	}
  | IDENTIFIER ARROW K_open
      { named_expr_t*tmp = new named_expr_t(lex_strings.make($1), 0);
	delete[]$1;
	$$ = tmp;
      }
  | IDENTIFIER ARROW error
      { errormsg(@3, "Invalid target for port map association.\n");
	yyerrok;
	$$ = 0;
      }
  ;

association_list
  : association_list ',' association_element
      { std::list<named_expr_t*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | association_element
      { std::list<named_expr_t*>*tmp = new std::list<named_expr_t*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

binding_indication
  : K_use entity_aspect_opt port_map_aspect_opt generic_map_aspect_opt
      { $$ = $2;
	if ($3) sorrymsg(@3, "Port map aspect not supported here. (binding_indication)\n");
	if ($4) sorrymsg(@4, "Generic map aspect not supported here. (binding_indication)\n");
	delete $3;
	delete $4;
      }
  ;

binding_indication_semicolon_opt
  : binding_indication ';' { $$ = $1; }
  | { $$ = 0; }
  ;

block_configuration
  : K_for IDENTIFIER
    use_clauses_opt
    configuration_items_opt
    K_end K_for ';'
    { delete[] $2; }
 ;

block_configuration_opt
  : block_configuration
  |
  ;

block_declarative_item
  : K_signal identifier_list ':' subtype_indication
    signal_declaration_assign_opt ';'
      { /* Save the signal declaration in the block_signals map. */
	for (std::list<perm_string>::iterator cur = $2->begin()
		   ; cur != $2->end() ; ++cur) {
	      Signal*sig = new Signal(*cur, $4, $5 ? $5->clone() : 0);
	      FILE_NAME(sig, @1);
	      active_scope->bind_name(*cur, sig);
	}
	delete $2;
      }

  | component_declaration

  | constant_declaration

  | subprogram_declaration

  | subprogram_body

  | subtype_declaration

  | type_declaration

  | use_clause_lib

      /* Various error handling rules for block_declarative_item... */

  | K_signal error ';'
      { errormsg(@2, "Syntax error declaring signals.\n"); yyerrok; }
  | error ';'
      { errormsg(@1, "Syntax error in block declarations.\n"); yyerrok; }

  ;

/*
 * The block_declarative_items rule matches "{ block_declarative_item }"
 * which is a synonym for "architecture_declarative_part" and
 * "block_declarative_part".
 */
block_declarative_items
  : block_declarative_items block_declarative_item
  | block_declarative_item
  ;

block_declarative_items_opt
  : block_declarative_items
  |
  ;

case_statement
  : K_case expression K_is
    case_statement_alternative_list
    K_end K_case ';'
      { CaseSeqStmt* tmp = new CaseSeqStmt($2, $4);
	FILE_NAME(tmp, @1);
	delete $4;
	$$ = tmp;
      }
    ;

case_statement_alternative_list
  : case_statement_alternative_list case_statement_alternative
      { std::list<CaseSeqStmt::CaseStmtAlternative*>* tmp = $1;
	tmp->push_back($2);
	$$ = tmp;
      }
  | case_statement_alternative
      { std::list<CaseSeqStmt::CaseStmtAlternative*>*tmp
		  = new std::list<CaseSeqStmt::CaseStmtAlternative*>();
	tmp->push_back($1);
	$$ = tmp;
      }
   ;

/*
 * The case_statement_alternative uses the "choice" rule to select the
 * reference expression, but that rule is shared with the aggregate
 * expression. So convert the ExpAggregate::choice_t to a case
 * statement alternative and pass that up instead.
 */
case_statement_alternative
  : K_when choices ARROW sequence_of_statements
      { CaseSeqStmt::CaseStmtAlternative* tmp;
        std::list<ExpAggregate::choice_t*>*choices = $2;
        std::list<Expression*>*exp_list = new std::list<Expression*>;
        bool others = false;

        for(std::list<ExpAggregate::choice_t*>::iterator it = choices->begin();
                it != choices->end(); ++it) {
            if((*it)->others() || others)
                // If there is one "others", then it also covers all other alternatives
                // Continue the loop to delete every choice_t, but do not
                // bother to add the expressions to the exp_list (we are going to
                // delete them very soon)
                others = true;
            else
                exp_list->push_back((*it)->simple_expression());

            delete (*it);
        }

        if(others) {
            tmp = new CaseSeqStmt::CaseStmtAlternative(0, $4);
            for(std::list<Expression*>::iterator it = exp_list->begin();
                    it != exp_list->end(); ++it) {
                delete (*it);
            }
        } else {
            tmp = new CaseSeqStmt::CaseStmtAlternative(exp_list, $4);
        }
        if (tmp) FILE_NAME(tmp, @1);

        delete choices;
        delete $4;
        $$ = tmp;
      }
   ;

choice
  : simple_expression
      { $$ = new ExpAggregate::choice_t($1);}
  | K_others
      { $$ = new ExpAggregate::choice_t; }
  | range /* discrete_range: range */
      { $$ = new ExpAggregate::choice_t($1); }
  ;

choices
  : choices '|' choice
      { std::list<ExpAggregate::choice_t*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | choice
      { std::list<ExpAggregate::choice_t*>*tmp = new std::list<ExpAggregate::choice_t*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

component_configuration
  : K_for component_specification
    binding_indication_semicolon_opt
    block_configuration_opt
    K_end K_for ';'
      {
    sorrymsg(@1, "Component configuration in not yet supported\n");
    if($3) delete $3;
    delete $2;
      }
  | K_for component_specification error K_end K_for
      {
    errormsg(@1, "Error in component configuration statement.\n");
    delete $2;
      }
  ;

component_declaration
  : K_component IDENTIFIER K_is_opt
    generic_clause_opt port_clause_opt
    K_end K_component identifier_opt ';'
      { perm_string name = lex_strings.make($2);
	if($8 && name != $8) {
	      errormsg(@8, "Identifier %s doesn't match component name %s.\n",
		       $8, name.str());
	}

	ComponentBase*comp = new ComponentBase(name);
	comp->set_interface($4, $5);
	if ($4) delete $4;
	if ($5) delete $5;

	active_scope->bind_name(name, comp);
	delete[]$2;
	if ($8) delete[] $8;
      }

  | K_component IDENTIFIER K_is_opt error K_end K_component identifier_opt ';'
      { errormsg(@4, "Syntax error in component declaration.\n");
	delete[] $2;
	if($7) delete[] $7;
	yyerrok;
      }
  ;

instantiated_unit
  : IDENTIFIER
  | K_component IDENTIFIER { $$ = $2; }
  ;

component_instantiation_statement
  : IDENTIFIER ':' instantiated_unit generic_map_aspect_opt port_map_aspect_opt ';'
      { perm_string iname = lex_strings.make($1);
	perm_string cname = lex_strings.make($3);
	ComponentInstantiation*tmp = new ComponentInstantiation(iname, cname, $4, $5);
	delete $4;
	delete $5;
	FILE_NAME(tmp, @1);
	delete[]$1;
	delete[]$3;
	$$ = tmp;
      }
  | IDENTIFIER ':' instantiated_unit error ';'
      { errormsg(@4, "Errors in component instantiation.\n");
	delete[]$1;
	delete[]$3;
	$$ = 0;
      }
  ;

component_specification
  : instantiation_list ':' name
      {
    ExpName* name = dynamic_cast<ExpName*>($3);
    std::pair<instant_list_t*, ExpName*>* tmp = new std::pair<instant_list_t*, ExpName*>($1, name);
    $$ = tmp;
      }
  ;

composite_type_definition
  /* constrained_array_definition */
  : K_array index_constraint K_of subtype_indication
      { VTypeArray*tmp = new VTypeArray($4, $2);
	delete $2;
	$$ = tmp;
      }

  /* unbounded_array_definition IEEE 1076-2008 P5.3.2.1 */
  | K_array '(' index_subtype_definition_list ')' K_of subtype_indication
      { std::list<ExpRange*> r;
	// NULL boundaries indicate unbounded array type
	ExpRange*tmp = new ExpRange(NULL, NULL, ExpRange::DOWNTO);
	r.push_back(tmp);
	FILE_NAME(tmp, @1);
	VTypeArray*arr = new VTypeArray($6, &r);
	$$ = arr;
      }

  | record_type_definition
      { $$ = $1; }
  ;

concurrent_assertion_statement
  : assertion_statement
      {
        /* See more explanations at IEEE 1076-2008 P11.5 */
        std::list<SequentialStmt*> stmts;
        stmts.push_back($1);
        stmts.push_back(new WaitStmt(WaitStmt::FINAL, NULL));
        push_scope();
        ProcessStatement*tmp = new ProcessStatement(empty_perm_string, *active_scope,
                                                    NULL, &stmts);
        pop_scope();
        FILE_NAME(tmp, @1);
        $$ = tmp;
      }
  ;

  /* The when...else..when...else syntax is not a general expression
     in VHDL but a specific sort of assignment statement model. We
     create Expression objects for it, but the parser will only
     recognize it it in specific situations. */
concurrent_conditional_signal_assignment /* IEEE 1076-2008 P11.6 */
  : name LEQ waveform K_when expression else_when_waveforms_opt ';'
      { std::list<ExpConditional::case_t*>*options;
        options = $6 ? $6 : new std::list<ExpConditional::case_t*>;
        options->push_front(new ExpConditional::case_t($5, $3));

        ExpName*name = dynamic_cast<ExpName*>($1);
        assert(name);
        CondSignalAssignment*tmp = new CondSignalAssignment(name, *options);

        FILE_NAME(tmp, @1);
        delete options;
        $$ = tmp;
      }

  /* Error recovery rules. */

  | name LEQ error K_when expression else_when_waveforms ';'
      { errormsg(@3, "Syntax error in waveform of conditional signal assignment.\n");
	ExpConditional*tmp = new ExpConditional($5, 0, $6);
	FILE_NAME(tmp, @3);
	delete $6;

        ExpName*name = dynamic_cast<ExpName*> ($1);
	assert(name);
	SignalAssignment*tmpa = new SignalAssignment(name, tmp);
	FILE_NAME(tmpa, @1);

	$$ = tmpa;
      }
  ;

concurrent_simple_signal_assignment
  : name LEQ waveform ';'
      { ExpName*name = dynamic_cast<ExpName*> ($1);
	assert(name);
	SignalAssignment*tmp = new SignalAssignment(name, *$3);
	FILE_NAME(tmp, @1);

	$$ = tmp;
	delete $3;
      }

else_when_waveforms
  : else_when_waveforms else_when_waveform
      { list<ExpConditional::case_t*>*tmp = $1;
	tmp ->push_back($2);
	$$ = tmp;
      }
  | else_when_waveform
      { list<ExpConditional::case_t*>*tmp = new list<ExpConditional::case_t*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

else_when_waveforms_opt
  : else_when_waveforms { $$ = $1; }
  | { $$ = 0; }
  ;


else_when_waveform
  : K_else waveform K_when expression
      { ExpConditional::case_t*tmp = new ExpConditional::case_t($4, $2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_else waveform
      { ExpConditional::case_t*tmp = new ExpConditional::case_t(0,  $2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  ;

concurrent_signal_assignment_statement /* IEEE 1076-2008 P11.6 */
  : concurrent_simple_signal_assignment

  | IDENTIFIER ':' concurrent_simple_signal_assignment
      { delete[] $1;
	$$ = $3;
      }

  | concurrent_conditional_signal_assignment

  | IDENTIFIER ':' concurrent_conditional_signal_assignment
      { delete[] $1;
	$$ = $3;
      }

  | selected_signal_assignment

  | IDENTIFIER ':' selected_signal_assignment { $$ = $3; }

  | name LEQ error ';'
      { errormsg(@2, "Syntax error in signal assignment waveform.\n");
	delete $1;
	$$ = 0;
	yyerrok;
      }
  | error LEQ waveform ';'
      { errormsg(@1, "Syntax error in l-value of signal assignment.\n");
	yyerrok;
	delete $3;
	$$ = 0;
      }
  ;

concurrent_statement
  : component_instantiation_statement
  | concurrent_signal_assignment_statement
  | concurrent_assertion_statement
  | generate_statement
  | process_statement
  ;

configuration_declaration
  : K_configuration IDENTIFIER K_of IDENTIFIER K_is
  configuration_declarative_part
  block_configuration
  K_end K_configuration_opt identifier_opt ';'
     {
  if(design_entities.find(lex_strings.make($4)) == design_entities.end())
      errormsg(@4, "Couldn't find entity %s used in configuration declaration\n", $4);
  //choose_architecture_for_entity();
  sorrymsg(@1, "Configuration declaration is not yet supported.\n");
     }
  | K_configuration error K_end K_configuration_opt identifier_opt ';'
      { errormsg(@2, "Too many errors, giving up on configuration declaration.\n");
    if($5) delete $5;
    yyerrok;
      }
  ;
//TODO: this list is only a sketch. It must be filled out later
configuration_declarative_item
  : use_clause
  ;

configuration_declarative_items
  : configuration_declarative_items configuration_declarative_item
  | configuration_declarative_item
  ;

configuration_declarative_part
  : configuration_declarative_items
  |
  ;

configuration_item
  : block_configuration
  | component_configuration
  ;

configuration_items
  : configuration_items configuration_item
  | configuration_item
  ;

configuration_items_opt
  : configuration_items
  |
  ;

constant_declaration
  : K_constant identifier_list ':' subtype_indication VASSIGN expression ';'
      { // The syntax allows multiple names to have the same type/value.
	for (std::list<perm_string>::iterator cur = $2->begin()
		   ; cur != $2->end() ; ++cur) {
	      active_scope->bind_name(*cur, $4, $6);
	}
	delete $2;
      }
  | K_constant identifier_list ':' subtype_indication ';'
      { sorrymsg(@1, "Deferred constant declarations not supported\n");
	delete $2;
      }

  /* Some error handling... */

  | K_constant identifier_list ':' subtype_indication VASSIGN error ';'
      { // The syntax allows multiple names to have the same type/value.
	errormsg(@6, "Error in value expression for constants.\n");
	yyerrok;
	for (std::list<perm_string>::iterator cur = $2->begin()
		   ; cur != $2->end() ; ++cur) {
	      active_scope->bind_name(*cur, $4, 0);
	}
	delete $2;
      }
  | K_constant identifier_list ':' error ';'
      { errormsg(@4, "Syntax error in constant declaration type.\n");
	yyerrok;
	delete $2;
      }
  | K_constant error ';'
      { errormsg(@2, "Syntax error in constant declaration.\n");
	yyerrok;
      }

  ;

context_clause : context_items | ;

context_item
  : library_clause
  | use_clause_lib
  ;

context_items
  : context_items context_item
  | context_item
  ;


design_unit
  : context_clause library_unit
  | error { errormsg(@1, "Invalid design_unit\n"); }
  ;

design_units
  : design_units design_unit
  | design_unit
  ;

direction
  : K_to { $$ = ExpRange::TO; }
  | K_downto { $$ = ExpRange::DOWNTO; }
  ;

element_association
  : choices ARROW expression
      { ExpAggregate::element_t*tmp = new ExpAggregate::element_t($1, $3);
	delete $1;
	$$ = tmp;
      }
  | expression
      { ExpAggregate::element_t*tmp = new ExpAggregate::element_t(0, $1);
	$$ = tmp;
      }
  ;

element_association_list
  : element_association_list ',' element_association
      { std::list<ExpAggregate::element_t*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | element_association
      { std::list<ExpAggregate::element_t*>*tmp = new std::list<ExpAggregate::element_t*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

element_declaration
  : identifier_list ':' subtype_indication ';'
      { $$ = record_elements($1, $3);
        delete $1;
      }
  ;

element_declaration_list
  : element_declaration_list element_declaration
      { $$ = $1;
	$$->splice($$->end(), *$2);
	delete $2;
      }
  | element_declaration
      { $$ = $1; }
  ;

  /* As an entity is declared, add it to the map of design entities. */
entity_aspect
  : K_entity name
      {
    ExpName* name = dynamic_cast<ExpName*>($2);
    entity_aspect_t* tmp = new entity_aspect_t(entity_aspect_t::ENTITY, name);
    $$ = tmp;
      }
  | K_configuration name
      {
    ExpName* name = dynamic_cast<ExpName*>($2);
    entity_aspect_t* tmp = new entity_aspect_t(entity_aspect_t::CONFIGURATION, name);
    $$ = tmp;
      }
  | K_open
      {
    entity_aspect_t* tmp = new entity_aspect_t(entity_aspect_t::OPEN, 0);
    $$ = tmp;
      }
  ;

entity_aspect_opt
  : entity_aspect { $$ = $1; }
  | { $$ = 0; }
  ;

entity_declaration
  : K_entity IDENTIFIER
    K_is generic_clause_opt port_clause_opt
    K_end K_entity_opt identifier_opt';'
      { Entity*tmp = new Entity(lex_strings.make($2));
	FILE_NAME(tmp, @1);
	  // Transfer the ports
	tmp->set_interface($4, $5);
	delete $4;
	delete $5;
	  // Save the entity in the entity map.
	design_entities[tmp->get_name()] = tmp;
	delete[]$2;
	if($8 && tmp->get_name() != $8) {
	      errormsg(@1, "Syntax error in entity clause. Closing name doesn't match.\n");
        }
        delete[]$8;
      }
  | K_entity error K_end K_entity_opt identifier_opt ';'
      { errormsg(@1, "Too many errors, giving up on entity declaration.\n");
	yyerrok;
	if ($5) delete[]$5;
      }
  ;

enumeration_literal
  : IDENTIFIER
      { list<perm_string>*tmp = new list<perm_string>;
	tmp->push_back(lex_strings.make($1));
	delete[]$1;
	$$ = tmp;
      }
  ;

enumeration_literal_list
  : enumeration_literal_list ',' enumeration_literal
      { list<perm_string>*tmp = $1;
	list<perm_string>*tmp3 = $3;
	if (tmp3) {
	      tmp->splice(tmp->end(), *tmp3);
	      delete tmp3;
	}
	$$ = tmp;
      }
  | enumeration_literal
      { list<perm_string>*tmp = $1;
	$$ = tmp;
      }
  ;

expression_list
  : expression_list ',' expression
      { list<Expression*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | expression
      { list<Expression*>*tmp = new list<Expression*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

expression
  : expression_logical
      { $$ = $1; }
  | range
      { $$ = $1; }
  ;

/*
 * The expression_logical matches the logical_expression from the
 * standard. Note that this is a little more tricky then usual because
 * of the strange VHDL rules for logical expressions. We have to
 * account for things like this:
 *
 *        <exp> and <exp> and <exp>...
 *
 * which is matched by the standard rule:
 *
 *        logical_expression ::=
 *           ...
 *             relation { and relation }
 *
 * The {} is important, and implies that "and" can be strung together
 * with other "and" operators without parentheses. This is true for
 * "and", "or", "xor", and "xnor". The tricky part is that these
 * cannot be mixed. For example, this is not OK:
 *
 *        <exp> and <exp> or <exp>
 *
 * Also note that "nand" and "nor" can not be chained in this manner.
 */
expression_logical
  : relation { $$ = $1; }
  | relation K_and expression_logical_and
      { ExpLogical*tmp = new ExpLogical(ExpLogical::AND, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | relation K_or expression_logical_or
      { ExpLogical*tmp = new ExpLogical(ExpLogical::OR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | relation K_xor expression_logical_xor
      { ExpLogical*tmp = new ExpLogical(ExpLogical::XOR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | relation K_nand relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::NAND, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | relation K_nor relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::NOR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | relation K_xnor expression_logical_xnor
      { ExpLogical*tmp = new ExpLogical(ExpLogical::XNOR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

expression_logical_and
  : relation
      { $$ = $1; }
  | expression_logical_and K_and relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::AND, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

expression_logical_or
  : relation
      { $$ = $1; }
  | expression_logical_or K_or relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::OR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

expression_logical_xnor
  : relation
      { $$ = $1; }
  | expression_logical_xnor K_xnor relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::XNOR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

expression_logical_xor
  : relation
      { $$ = $1; }
  | expression_logical_xor K_xor relation
      { ExpLogical*tmp = new ExpLogical(ExpLogical::XOR, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

factor
  : primary
      { $$ = $1; }
  | primary EXP primary
      { ExpArithmetic*tmp = new ExpArithmetic(ExpArithmetic::POW, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | K_abs primary
      { ExpUAbs*tmp = new ExpUAbs($2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_not primary
      { ExpUNot*tmp = new ExpUNot($2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  ;

file_declaration
  : K_file identifier_list ':' IDENTIFIER file_open_information_opt ';'
      {
	if (strcasecmp($4, "TEXT"))
	      sorrymsg(@1, "file declaration currently handles only TEXT type.\n");

	for (std::list<perm_string>::iterator cur = $2->begin()
		   ; cur != $2->end() ; ++cur) {
	      Variable*var = new Variable(*cur, &primitive_INTEGER);
	      FILE_NAME(var, @1);
	      active_scope->bind_name(*cur, var);

	      // there was a file name specified, so it needs an implicit call
	      // to open it at the beginning of simulation and close it at the end
	      if($5) {
		std::list<Expression*> params;

		// add file_open() call in 'initial' block
		params.push_back(new ExpScopedName(active_scope->peek_name(), new ExpName(*cur)));
		params.push_back($5->filename()->clone());
		params.push_back($5->kind()->clone());
		ProcedureCall*fopen_call = new ProcedureCall(
                                    perm_string::literal("file_open"), &params);
		arc_scope->add_initializer(fopen_call);

		// add file_close() call in 'final' block
		params.clear();
		params.push_back(new ExpScopedName(active_scope->peek_name(), new ExpName(*cur)));
		ProcedureCall*fclose_call = new ProcedureCall(
                                    perm_string::literal("file_close"), &params);
		arc_scope->add_finalizer(fclose_call);

		delete $5;
	      }
	}

	delete $2;
      }
  | K_file error ';'
      { errormsg(@2, "Syntax error in file declaration.\n");
	yyerrok;
      }
  ;

file_open_information
  : K_open IDENTIFIER K_is STRING_LITERAL
     {
        ExpName*mode = new ExpName(lex_strings.make($2));
        delete[]$2;
        FILE_NAME(mode, @1);
        $$ = new file_open_info_t(new ExpString($4), mode);
     }
  | K_is STRING_LITERAL
     {
        $$ = new file_open_info_t(new ExpString($2));
     }

file_open_information_opt
  : file_open_information { $$ = $1; }
  | { $$ = 0; }
  ;

for_generate_statement
  : IDENTIFIER ':' K_for IDENTIFIER K_in range
    K_generate generate_statement_body
    K_end K_generate identifier_opt ';'
      { perm_string name = lex_strings.make($1);
	perm_string gvar = lex_strings.make($4);
	ForGenerate*tmp = new ForGenerate(name, gvar, $6, *$8);
	FILE_NAME(tmp, @1);

	if ($11 && name != $11) {
	      errormsg(@1, "for-generate name %s does not match closing name %s\n",
		       name.str(), $11);
	}
	delete[]$1;
	delete[]$4;
	delete $8;
	delete[]$11;
	$$ = tmp;
      }
  ;

function_specification /* IEEE 1076-2008 P4.2.1 */
  : K_function IDENTIFIER parameter_list K_return IDENTIFIER
      { perm_string type_name = lex_strings.make($5);
	perm_string name = lex_strings.make($2);
	const VType*type_mark = active_scope->find_type(type_name);
	touchup_interface_for_functions($3);
	SubprogramHeader*tmp = new SubprogramHeader(name, $3, type_mark);
	FILE_NAME(tmp, @1);
	delete[]$2;
	delete[]$5;
	$$ = tmp;
      }
  ;

generate_statement /* IEEE 1076-2008 P11.8 */
  : if_generate_statement
  | for_generate_statement
  ;

generate_statement_body
  : architecture_statement_part { $$ = $1; }
  ;

generic_clause_opt
  : generic_clause
      { $$ = $1;}
  |
      { $$ = 0; }
  ;

generic_clause
  : K_generic parameter_list ';'
      { $$ = $2; }
  | K_generic '(' error ')' ';'
      { errormsg(@3, "Error in interface list for generic.\n");
	yyerrok;
	$$ = 0;
      }
  ;

generic_map_aspect_opt
  : generic_map_aspect { $$ = $1; }
  |                    { $$ = 0; }
  ;

generic_map_aspect
  : K_generic K_map '(' association_list ')'
      { $$ = $4; }
  | K_generic K_map '(' error ')'
      { errormsg(@4, "Error in association list for generic map.\n");
	yyerrok;
	$$ = 0;
      }
  ;

identifier_list
  : identifier_list ',' IDENTIFIER
      { std::list<perm_string>* tmp = $1;
	tmp->push_back(lex_strings.make($3));
	delete[]$3;
	$$ = tmp;
      }
  | IDENTIFIER
      { std::list<perm_string>*tmp = new std::list<perm_string>;
	tmp->push_back(lex_strings.make($1));
	delete[]$1;
	$$ = tmp;
      }
  ;

identifier_opt : IDENTIFIER { $$ = $1; } | { $$ = 0; } ;

identifier_colon_opt : IDENTIFIER ':' { $$ = $1; } | { $$ = 0; };

  /* The if_generate_statement rule describes the if_generate syntax.

     NOTE: This does not yet implement the elsif and else parts of the
     syntax. This shouldn't be hard, but is simply not done yet. */
if_generate_statement /* IEEE 1076-2008 P11.8 */
  : IDENTIFIER ':' K_if expression
    K_generate generate_statement_body
    K_end K_generate identifier_opt ';'
      { perm_string name = lex_strings.make($1);
	IfGenerate*tmp = new IfGenerate(name, $4, *$6);
	FILE_NAME(tmp, @3);

	if ($9 && name != $9) {
	      errormsg(@1, "if-generate name %s does not match closing name %s\n",
		       name.str(), $9);
	}
	delete[]$1;
	delete  $6;
	delete[]$9;
	$$ = tmp;
      }
  ;

if_statement
  : K_if expression K_then sequence_of_statements
    if_statement_elsif_list_opt if_statement_else
    K_end K_if ';'
      { IfSequential*tmp = new IfSequential($2, $4, $5, $6);
	FILE_NAME(tmp, @1);
	delete $4;
	delete $5;
	delete $6;
	$$ = tmp;
      }

  | K_if error K_then sequence_of_statements
    if_statement_elsif_list_opt if_statement_else
    K_end K_if ';'
      { errormsg(@2, "Error in if_statement condition expression.\n");
	yyerrok;
	$$ = 0;
	delete $4;
      }

  | K_if expression K_then error K_end K_if ';'
      { errormsg(@4, "Too many errors in sequence within if_statement.\n");
	yyerrok;
	$$ = 0;
      }

  | K_if error K_end K_if ';'
      { errormsg(@2, "Too many errors in if_statement.\n");
	yyerrok;
	$$ = 0;
      }
  ;

if_statement_elsif_list_opt
  : if_statement_elsif_list { $$ = $1; }
  |                         { $$ = 0;  }
  ;

if_statement_elsif_list
  : if_statement_elsif_list if_statement_elsif
      { list<IfSequential::Elsif*>*tmp = $1;
	tmp->push_back($2);
	$$ = tmp;
      }
  | if_statement_elsif
      { list<IfSequential::Elsif*>*tmp = new list<IfSequential::Elsif*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

if_statement_elsif
  : K_elsif expression K_then sequence_of_statements
      { IfSequential::Elsif*tmp = new IfSequential::Elsif($2, $4);
	FILE_NAME(tmp, @1);
	delete $4;
	$$ = tmp;
      }
  | K_elsif expression K_then error
      { errormsg(@4, "Too many errors in elsif sub-statements.\n");
	yyerrok;
	$$ = 0;
      }
  ;

if_statement_else
  : K_else sequence_of_statements
      { $$ = $2; }
  | K_else error
      { errormsg(@2, "Too many errors in else sub-statements.\n");
	yyerrok;
	$$ = 0;
      }
  |
      { $$ = 0; }
  ;

index_constraint
  : '(' range_list ')'
      { $$ = $2; }
  | '(' error ')'
      { errormsg(@2, "Errors in the index constraint.\n");
	yyerrok;
	$$ = new list<ExpRange*>;
      }
  ;

  /* The identifier should be a type name */
index_subtype_definition /* IEEE 1076-2008 P5.3.2.1 */
  : IDENTIFIER K_range BOX
  ;

index_subtype_definition_list
  : index_subtype_definition_list ',' index_subtype_definition
  | index_subtype_definition
  ;

instantiation_list
  : identifier_list
     {
  instant_list_t* tmp = new instant_list_t(instant_list_t::NO_DOMAIN, $1);
  $$ = tmp;
     }
  | K_others
     {
  instant_list_t* tmp = new instant_list_t(instant_list_t::OTHERS, 0);
  $$ = tmp;
     }
  | K_all
   {
  instant_list_t* tmp = new instant_list_t(instant_list_t::ALL, 0);
  $$ = tmp;
   }
  ;

  /* The interface_element is also an interface_declaration */
interface_element
  : identifier_list ':' mode_opt subtype_indication interface_element_expression
      { std::list<InterfacePort*>*tmp = new std::list<InterfacePort*>;
	for (std::list<perm_string>::iterator cur = $1->begin()
		   ; cur != $1->end() ; ++cur) {
	      InterfacePort*port = new InterfacePort;
	      FILE_NAME(port, @1);
	      port->mode = $3;
	      port->name = *(cur);
	      port->type = $4;
	      port->expr = $5;
	      tmp->push_back(port);
	}
	delete $1;
	$$ = tmp;
      }
  ;

interface_element_expression
  : VASSIGN expression { $$ = $2; }
  |                    { $$ = 0; }
  ;

interface_list
  : interface_list ';' interface_element
      { std::list<InterfacePort*>*tmp = $1;
	tmp->splice(tmp->end(), *$3);
	delete $3;
	$$ = tmp;
      }
  | interface_element
      { std::list<InterfacePort*>*tmp = $1;
	$$ = tmp;
      }
  ;

library_clause
  : K_library logical_name_list ';'
      { library_import(@1, $2);
	delete $2;
      }
  | K_library error ';'
     { errormsg(@1, "Syntax error in library clause.\n"); yyerrok; }
  ;

  /* Collapse the primary_unit and secondary_unit of the library_unit
     into this single set of rules. */
library_unit
  : primary_unit
  | secondary_unit
  ;

logical_name : IDENTIFIER { $$ = $1; } ;

logical_name_list
  : logical_name_list ',' logical_name
      { std::list<perm_string>*tmp = $1;
	tmp->push_back(lex_strings.make($3));
	delete[]$3;
	$$ = tmp;
      }
  | logical_name
      { std::list<perm_string>*tmp = new std::list<perm_string>;
	tmp->push_back(lex_strings.make($1));
	delete[]$1;
	$$ = tmp;
      }
  ;

loop_statement
  : identifier_colon_opt
    K_while expression_logical K_loop
    sequence_of_statements
    K_end K_loop identifier_opt ';'
      { perm_string loop_name = $1? lex_strings.make($1) : perm_string() ;
	if ($8 && !$1) {
	      errormsg(@8, "Loop statement closing name %s for un-named statement\n", $8);
	} else if($8 && loop_name != $8) {
	      errormsg(@1, "Loop statement name %s doesn't match closing name %s.\n", loop_name.str(), $8);
	}
	if($1) delete[]$1;
	if($8) delete[]$8;

	WhileLoopStatement* tmp = new WhileLoopStatement(loop_name, $3, $5);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }

  | identifier_colon_opt
    K_for IDENTIFIER K_in range
    K_loop sequence_of_statements K_end K_loop identifier_opt ';'
      { perm_string loop_name = $1? lex_strings.make($1) : perm_string() ;
	perm_string index_name = lex_strings.make($3);
	if ($10 && !$1) {
	      errormsg(@10, "Loop statement closing name %s for un-named statement\n", $10);
	} else if($10 && loop_name != $10) {
	      errormsg(@1, "Loop statement name %s doesn't match closing name %s.\n", loop_name.str(), $10);
	}
	if($1)  delete[] $1;
	delete[] $3;
	if($10) delete[] $10;

	ForLoopStatement* tmp = new ForLoopStatement(loop_name, index_name, $5, $7);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }

  | identifier_colon_opt
    K_loop sequence_of_statements K_end K_loop identifier_opt ';'
      { perm_string loop_name = $1? lex_strings.make($1) : perm_string() ;
	if ($6 && !$1) {
	      errormsg(@6, "Loop statement closing name %s for un-named statement\n", $6);
	} else if($6 && loop_name != $6) {
	      errormsg(@1, "Loop statement name %s doesn't match closing name %s.\n", loop_name.str(), $6);
	}
	if($1) delete[]$1;
	if($6) delete[]$6;

	BasicLoopStatement* tmp = new BasicLoopStatement(loop_name, $3);
	FILE_NAME(tmp, @2);

	$$ = tmp;
      };

mode
  : K_in  { $$ = PORT_IN; }
  | K_out { $$ = PORT_OUT; }
  | K_inout { $$ = PORT_INOUT; }
  ;

mode_opt : mode {$$ = $1;} | {$$ = PORT_NONE;} ;

name /* IEEE 1076-2008 P8.1 */
  : IDENTIFIER /* simple_name (IEEE 1076-2008 P8.2) */
      { Expression*tmp;
        /* Check if the IDENTIFIER is one of CHARACTER enums (LF, CR, etc.) */
        tmp = parse_char_enums($1);
        if(!tmp) {
            perm_string name = lex_strings.make($1);
            /* There are functions that have the same name types, e.g. integer */
            if(!active_scope->find_subprogram(name).empty() && !parse_type_by_name(name))
                tmp = new ExpFunc(name);
            else
                tmp = new ExpName(name);
        }
        FILE_NAME(tmp, @1);
        delete[]$1;
        $$ = tmp;
      }

  | selected_name
      { $$ = $1; }

  | indexed_name
      { $$ = $1; }

  | selected_name '(' expression_list ')'
    {
        ExpName*name = dynamic_cast<ExpName*>($1);
        assert(name);
        name->add_index($3);
        delete $3;  // contents of the list is moved to the selected_name
    }
  ;

indexed_name
  /* Note that this rule can match array element selects and various
     function calls. The only way we can tell the difference is from
     left context, namely whether the name is a type name or function
     name. If none of the above, treat it as a array element select. */
  : IDENTIFIER '(' expression_list ')'
      { Expression*tmp;
        perm_string name = lex_strings.make($1);
        delete[]$1;
        if (active_scope->is_vector_name(name) || is_subprogram_param(name))
            tmp = new ExpName(name, $3);
        else
            tmp = new ExpFunc(name, $3);
        FILE_NAME(tmp, @1);
        $$ = tmp;
      }
  | indexed_name '(' expression_list ')'
      { ExpName*name = dynamic_cast<ExpName*>($1);
        assert(name);
        name->add_index($3);
        $$ = $1;
      }
  ;

  /* Handle name lists as lists of expressions. */
name_list
  : name_list ',' name
      { std::list<Expression*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | name
      { std::list<Expression*>*tmp = new std::list<Expression*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

package_declaration
  : package_declaration_start K_is
    package_declarative_part_opt
    K_end K_package_opt identifier_opt ';'
      { perm_string name = lex_strings.make($1);
	if($6 && name != $6) {
	      errormsg(@1, "Identifier %s doesn't match package name %s.\n",
		       $6, name.str());
        }
	Package*tmp = new Package(name, *active_scope);
	FILE_NAME(tmp, @1);
	delete[]$1;
        if ($6) delete[]$6;
	pop_scope();
	  /* Put this package into the work library, or the currently
	     parsed library. Note that parse_library_name is an
	     argument to the parser. */
	library_save_package(parse_library_name, tmp);
      }
  | package_declaration_start K_is error K_end K_package_opt identifier_opt ';'
    { errormsg(@3, "Syntax error in package clause.\n");
      yyerrok;
      pop_scope();
    }
  ;

package_declaration_start
  : K_package IDENTIFIER
      { push_scope();
	$$ = $2;
      }
  ;

/* TODO: this list must be extended in the future
   presently it is only a sketch */
package_body_declarative_item /* IEEE1076-2008 P4.8 */
  : use_clause
  | subprogram_body
  ;

package_body_declarative_items
  : package_body_declarative_items package_body_declarative_item
  | package_body_declarative_item
  ;
package_body_declarative_part_opt
  : package_body_declarative_items
  |
  ;

package_declarative_item
  : component_declaration
  | constant_declaration
  | subprogram_declaration
  | subtype_declaration
  | type_declaration
  | use_clause
  | error ';'
      { errormsg(@1, "Syntax error in package declarative item.\n");
	yyerrok;
      }
  ;

package_declarative_items
  : package_declarative_items package_declarative_item
  | package_declarative_item
  ;

package_declarative_part_opt
  : package_declarative_items
  |
  ;

package_body /* IEEE 1076-2008 P4.8 */
  : package_body_start K_is
    package_body_declarative_part_opt
    K_end K_package_opt identifier_opt ';'
      { perm_string name = lex_strings.make($1);
	if ($6 && name != $6)
	      errormsg(@6, "Package name (%s) doesn't match closing name (%s).\n", $1, $6);
	delete[] $1;
	if($6) delete[]$6;
	pop_scope();
      }

  | package_body_start K_is error K_end K_package_opt identifier_opt ';'
      { errormsg(@1, "Errors in package %s body.\n", $1);
	yyerrok;
	pop_scope();
      }
  ;

/*
 * This is a portion of the package_body rule that we factor out so
 * that we can use this rule to start the scope.
 */
package_body_start
  : K_package K_body IDENTIFIER
      { perm_string name = lex_strings.make($3);
	push_scope();
	Package*pkg = library_recall_package(parse_library_name, name);
	if (pkg != 0) {
	      active_scope->set_package_header(pkg);
	} else {
	      errormsg(@1, "Package body for %s has no matching header.\n", $3);
	}
	$$ = $3;
      }
  ;

parameter_list
  : '(' interface_list ')' { $$ = $2; }
  ;

parameter_list_opt
  : parameter_list  { $$ = $1; }
  |                 { $$ = 0; }
  ;

port_clause
  : K_port parameter_list ';'
      { $$ = $2; }
  | K_port '(' error ')' ';'
      { errormsg(@1, "Syntax error in port list.\n");
	yyerrok;
	$$ = 0;
      }
  ;

port_clause_opt : port_clause {$$ = $1;} | {$$ = 0;} ;

port_map_aspect
  : K_port K_map '(' association_list ')'
      { $$ = $4; }
  | K_port K_map '(' error ')'
      { errormsg(@1, "Syntax error in port map aspect.\n");
	yyerrok;
      }
  ;

port_map_aspect_opt
  : port_map_aspect  { $$ = $1; }
  |                  { $$ = 0; }
  ;


prefix /* IEEE 1076-2008 P8.1 */
  : name
      { $$ = $1; }
  ;

primary
  : name
      { $$ = $1; }
  | name '\'' IDENTIFIER argument_list_opt
      { ExpAttribute*tmp = NULL;
	perm_string attr = lex_strings.make($3);
	ExpName*base = dynamic_cast<ExpName*>($1);
	const VType*type = parse_type_by_name(base->peek_name());

	if(type) {
	    tmp = new ExpTypeAttribute(type, attr, $4);
	} else {
	    tmp = new ExpObjAttribute(base, attr, $4);
	}

	FILE_NAME(tmp, @1);
	delete[]$3;
	$$ = tmp;
      }
  | CHARACTER_LITERAL
      { ExpCharacter*tmp = new ExpCharacter($1[0]);
	FILE_NAME(tmp,@1);
	delete[]$1;
	$$ = tmp;
      }
  | INT_LITERAL
      { ExpInteger*tmp = new ExpInteger($1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | REAL_LITERAL
      { ExpReal*tmp = new ExpReal($1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | STRING_LITERAL
      { ExpString*tmp = new ExpString($1);
	FILE_NAME(tmp,@1);
	delete[]$1;
	$$ = tmp;
      }
  | BITSTRING_LITERAL
      { ExpBitstring*tmp = new ExpBitstring($1);
	FILE_NAME(tmp, @1);
	delete[]$1;
	$$ = tmp;
      }
  | INT_LITERAL IDENTIFIER
      { ExpTime::timeunit_t unit = ExpTime::FS;

        if(!strcasecmp($2, "us"))
            unit = ExpTime::US;
        else if(!strcasecmp($2, "ms"))
            unit = ExpTime::MS;
        else if(!strcasecmp($2, "ns"))
            unit = ExpTime::NS;
        else if(!strcasecmp($2, "s"))
            unit = ExpTime::S;
        else if(!strcasecmp($2, "ps"))
            unit = ExpTime::PS;
        else if(!strcasecmp($2, "fs"))
            unit = ExpTime::FS;
        else
            errormsg(@2, "Invalid time unit (accepted are fs, ps, ns, us, ms, s).\n");

        if($1 < 0)
            errormsg(@1, "Time cannot be negative.\n");

        ExpTime*tmp = new ExpTime($1, unit);
        FILE_NAME(tmp, @1);
        delete[] $2;
        $$ = tmp;
      }

/*XXXX Caught up in element_association_list?
  | '(' expression ')'
      { $$ = $2; }
*/
  /* This catches function calls that use association lists for the
     argument list. The position argument list is discovered elsewhere
     and must be discovered by elaboration (thanks to the ambiguity of
     VHDL syntax). */
  | IDENTIFIER '(' association_list ')'
      { sorrymsg(@1, "Function calls not supported\n");
	delete[] $1;
	$$ = 0;
      }

  /* Aggregates */

  | '(' element_association_list ')'
      { Expression*tmp = aggregate_or_primary(@1, $2);
	$$ = tmp;
      }
  ;

primary_unit
  : entity_declaration
  | configuration_declaration
  | package_declaration
  ;

procedure_call
  : IDENTIFIER ';'
      {
    ProcedureCall* tmp = new ProcedureCall(lex_strings.make($1));
    FILE_NAME(tmp, @1);
    delete[] $1;
    $$ = tmp;
      }
  | IDENTIFIER '(' association_list ')' ';'
      {
    ProcedureCall* tmp = new ProcedureCall(lex_strings.make($1), $3);
    FILE_NAME(tmp, @1);
    delete[] $1;
    $$ = tmp;
      }
  | IDENTIFIER argument_list ';'
      {
    ProcedureCall* tmp = new ProcedureCall(lex_strings.make($1), $2);
    FILE_NAME(tmp, @1);
    delete[] $1;
    delete $2; // parameters are copied in this variant
    $$ = tmp;
      }
  | IDENTIFIER '(' error ')' ';'
      { errormsg(@1, "Errors in procedure call.\n");
	yyerrok;
	delete[]$1;
	$$ = 0;
      }
  ;

procedure_call_statement
  : IDENTIFIER ':' procedure_call
      { delete[] $1;
	$$ = $3;
      }
  | procedure_call { $$ = $1; }
  ;

procedure_specification /* IEEE 1076-2008 P4.2.1 */
  : K_procedure IDENTIFIER parameter_list_opt
      { perm_string name = lex_strings.make($2);
	touchup_interface_for_functions($3);
	SubprogramHeader*tmp = new SubprogramHeader(name, $3, NULL);
	FILE_NAME(tmp, @1);
	delete[]$2;
	$$ = tmp;
      }
  ;

process_declarative_item
  : variable_declaration
  | file_declaration
  ;

process_declarative_part
  : process_declarative_part process_declarative_item
  | process_declarative_item
  ;

process_declarative_part_opt
  : process_declarative_part
  |
  ;

process_start
  : identifier_colon_opt K_postponed_opt K_process
       { push_scope();
         $$ = $1;
       }
  ;

process_statement
  : process_start process_sensitivity_list_opt K_is_opt
    process_declarative_part_opt
    K_begin sequence_of_statements
    K_end K_postponed_opt K_process identifier_opt ';'
      { perm_string iname = $1? lex_strings.make($1) : empty_perm_string;
	if ($1) delete[]$1;
	if ($10) {
	      if (iname.nil()) {
		    errormsg(@10, "Process end name %s for un-named processes.\n", $10);
	      } else if (iname != $10) {
		    errormsg(@10, "Process name %s does not match opening name %s.\n",
			     $10, $1);
	      }
	      delete[]$10;
	}

	ProcessStatement*tmp = new ProcessStatement(iname, *active_scope, $2, $6);
        arc_scope->bind_scope(tmp->peek_name(), tmp);
        pop_scope();
	FILE_NAME(tmp, @3);
	delete $2;
	delete $6;
	$$ = tmp;
      }

  | process_start process_sensitivity_list_opt K_is_opt
    process_declarative_part_opt
    K_begin error
    K_end K_postponed_opt K_process identifier_opt ';'
      { errormsg(@7, "Too many errors in process sequential statements.\n");
	yyerrok;
	$$ = 0;
      }
  ;

/*
 * A process_sensitivity_list is:
 *     <nil>  if the list is not present, or
 *     or a non-empty list of actual expressions.
 */
process_sensitivity_list_opt
  : '(' process_sensitivity_list ')'
      { $$ = $2; }
  | '(' error ')'
      { errormsg(@2, "Error in process sensitivity list\n");
	yyerrok;
	$$ = 0;
      }
  |
      { $$ = 0; }
  ;

process_sensitivity_list
  : K_all
      { std::list<Expression*>*tmp = new std::list<Expression*>;
	ExpName*all = new ExpNameALL;
	FILE_NAME(all, @1);
	tmp->push_back(all);
	$$ = tmp;
      }
  | name_list
      { $$ = $1; }
  ;

range
  : simple_expression direction simple_expression
      { ExpRange* tmp = new ExpRange($1, $3, $2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | name '\'' K_range
      {
        ExpRange*tmp = NULL;
        ExpName*name = NULL;
        if((name = dynamic_cast<ExpName*>($1))) {
            tmp = new ExpRange(name, false);
            FILE_NAME(tmp, @1);
        } else {
	    errormsg(@1, "'range attribute can be used with named expressions only");
        }
        $$ = tmp;
      }
  | name '\'' K_reverse_range
      {
        ExpRange*tmp = NULL;
        ExpName*name = NULL;
        if((name = dynamic_cast<ExpName*>($1))) {
            tmp = new ExpRange(name, true);
            FILE_NAME(tmp, @1);
        } else {
	    errormsg(@1, "'reverse_range attribute can be used with named expressions only");
        }
        $$ = tmp;
      }
  ;

range_list
  : range
      { list<ExpRange*>*tmp = new list<ExpRange*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  | range_list ',' range
      { list<ExpRange*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  ;

record_type_definition
  : K_record element_declaration_list K_end K_record
      { VTypeRecord*tmp = new VTypeRecord($2);
	$$ = tmp;
      }
  ;

relation
  : shift_expression
      { $$ = $1; }
  | shift_expression '=' shift_expression
      { ExpRelation*tmp = new ExpRelation(ExpRelation::EQ, $1, $3);
        FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | shift_expression '<' shift_expression
      { ExpRelation*tmp = new ExpRelation(ExpRelation::LT, $1, $3);
        FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | shift_expression '>' shift_expression
      { ExpRelation*tmp = new ExpRelation(ExpRelation::GT, $1, $3);
        FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | shift_expression LEQ shift_expression
      { ExpRelation*tmp = new ExpRelation(ExpRelation::LE, $1, $3);
        FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | shift_expression GEQ shift_expression
      { ExpRelation*tmp = new ExpRelation(ExpRelation::GE, $1, $3);
        FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | shift_expression NE shift_expression
      { ExpRelation*tmp = new ExpRelation(ExpRelation::NEQ, $1, $3);
        FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

report_statement
  : K_report expression severity_opt ';'
      { ReportStmt*tmp = new ReportStmt($2, $3);
	FILE_NAME(tmp,@2);
	$$ = tmp;
      }

return_statement
  : K_return expression ';'
      { ReturnStmt*tmp = new ReturnStmt($2);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_return ';'
      { ReturnStmt*tmp = new ReturnStmt(0);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_return error ';'
      { ReturnStmt*tmp = new ReturnStmt(0);
	FILE_NAME(tmp, @1);
	$$ = tmp;
	errormsg(@2, "Error in expression in return statement.\n");
	yyerrok;
      }
  ;

secondary_unit
  : architecture_body
  | package_body
  ;

selected_name /* IEEE 1076-2008 P8.3 */
  : prefix '.' suffix
      { Expression*pfx = $1;
	ExpName*pfx1 = dynamic_cast<ExpName*>(pfx);
	assert(pfx1);
	perm_string tmp = lex_strings.make($3);
	$$ = new ExpName(pfx1, tmp);
	FILE_NAME($$, @3);
	delete[]$3;
      }
  | error '.' suffix
      { errormsg(@1, "Syntax error in prefix in front of \"%s\".\n", $3);
        yyerrok;
	$$ = new ExpName(lex_strings.make($3));
	FILE_NAME($$, @3);
	delete[]$3;
      }
  ;

selected_names
  : selected_names ',' selected_name
      { std::list<Expression*>* tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | selected_name
      { std::list<Expression*>* tmp = new std::list<Expression*>();
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

  /* The *_lib variant of selected_name is used by the "use"
     clause. It is syntactically identical to other selected_name
     rules, but is a convenient place to attach use_clause actions. */
selected_name_lib
  : IDENTIFIER '.' K_all
      { library_use(@1, active_scope, 0, $1, 0);
	delete[]$1;
      }
  | IDENTIFIER '.' IDENTIFIER '.' K_all
      { library_use(@1, active_scope, $1, $3, 0);
	delete[]$1;
	delete[]$3;
      }
  | IDENTIFIER '.' IDENTIFIER '.' IDENTIFIER
      { library_use(@1, active_scope, $1, $3, $5);
	delete[]$1;
	delete[]$3;
	delete[]$5;
      }
  ;

selected_names_lib
  : selected_names_lib ',' selected_name_lib
  | selected_name_lib
  ;

selected_signal_assignment
  : K_with expression K_select name LEQ selected_waveform_list ';'
      { ExpSelected*tmp = new ExpSelected($2, $6);
	FILE_NAME(tmp, @3);
        delete $2;
	delete $6;

	ExpName*name = dynamic_cast<ExpName*>($4);
	assert(name);
	SignalAssignment*tmpa = new SignalAssignment(name, tmp);
	FILE_NAME(tmpa, @1);

	$$ = tmpa;
      }
  ;

selected_waveform
  : waveform K_when expression
      { ExpConditional::case_t*tmp = new ExpConditional::case_t($3, $1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | waveform K_when K_others
      { ExpConditional::case_t*tmp = new ExpConditional::case_t(0,  $1);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  ;

selected_waveform_list
  : selected_waveform_list ',' selected_waveform
      { list<ExpConditional::case_t*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | selected_waveform
      { list<ExpConditional::case_t*>*tmp = new list<ExpConditional::case_t*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

list_of_statements
  : list_of_statements sequential_statement
      { std::list<SequentialStmt*>*tmp = $1;
    if($2)
        tmp->push_back($2);
	$$ = tmp;
      }
  | sequential_statement
      { std::list<SequentialStmt*>*tmp = new std::list<SequentialStmt*>;
    if($1)
        tmp->push_back($1);
	$$ = tmp;
      }
  ;

sequence_of_statements
  : list_of_statements { $1 = $$; }
  | { $$ = NULL; }
  ;

sequential_statement
  : if_statement                { $$ = $1; }
  | signal_assignment_statement { $$ = $1; }
  | variable_assignment_statement { $$ = $1; }
  | case_statement { $$ = $1; }
  | procedure_call_statement { $$ = $1; }
  | loop_statement { $$ = $1; }
  | return_statement { $$ = $1; }
  | report_statement { $$ = $1; }
  | assertion_statement { $$ = $1; }
  | wait_statement { $$ = $1; }
  | K_null ';' { $$ = 0; }
  | error ';'
      { errormsg(@1, "Syntax error in sequential statement.\n");
	$$ = 0;
	yyerrok;
      }
  ;

severity
  : K_severity IDENTIFIER
  { if(!strcasecmp($2, "NOTE"))
        $$ = ReportStmt::NOTE;
    else if(!strcasecmp($2, "WARNING"))
        $$ = ReportStmt::WARNING;
    else if(!strcasecmp($2, "ERROR"))
        $$ = ReportStmt::ERROR;
    else if(!strcasecmp($2, "FAILURE"))
        $$ = ReportStmt::FAILURE;
    else {
        errormsg(@1, "Invalid severity level (possible values: NOTE, WARNING, ERROR, FAILURE).\n");
        $$ = ReportStmt::UNSPECIFIED;
    }
    delete[] $2;
  }

severity_opt
  : severity { $$ = $1; }
  | { $$ = ReportStmt::UNSPECIFIED; }

shift_expression
  : simple_expression
  | simple_expression K_srl simple_expression
      { ExpShift*tmp = new ExpShift(ExpShift::SRL, $1, $3);
        FILE_NAME(tmp, @2);
        $$ = tmp;
      }
  | simple_expression K_sll simple_expression
      { ExpShift*tmp = new ExpShift(ExpShift::SLL, $1, $3);
        FILE_NAME(tmp, @2);
        $$ = tmp;
      }
  | simple_expression K_sra simple_expression
      { ExpShift*tmp = new ExpShift(ExpShift::SRA, $1, $3);
        FILE_NAME(tmp, @2);
        $$ = tmp;
      }
  | simple_expression K_sla simple_expression
      { ExpShift*tmp = new ExpShift(ExpShift::SLA, $1, $3);
        FILE_NAME(tmp, @2);
        $$ = tmp;
      }
  | simple_expression K_ror simple_expression
      { sorrymsg(@2, "ROR is not supported.\n");
        ExpShift*tmp = new ExpShift(ExpShift::ROR, $1, $3);
        FILE_NAME(tmp, @2);
        $$ = tmp;
      }
  | simple_expression K_rol simple_expression
      { sorrymsg(@2, "ROL is not supported.\n");
        ExpShift*tmp = new ExpShift(ExpShift::ROL, $1, $3);
        FILE_NAME(tmp, @2);
        $$ = tmp;
      }
  ;

signal_declaration_assign_opt
  : VASSIGN expression { $$ = $2; }
  |                    { $$ = 0;  }
  ;

/*
 * The LRM rule for simple_expression is:
 *   simple_expression ::= [sign] term { adding_operator term }
 *
 * This is functionally a list of terms, with the adding_operator used
 * as a list element separator instead of a ','. The LRM rule,
 * however, is right-recursive, which is not too nice in real LALR
 * parsers. The solution is to rewrite it as below, to make it
 * left-recursive. This is much more efficient use of the parse stack.
 *
 * Note that although the concatenation operator '&' is syntactically
 * an addition operator, it is handled differently during elaboration
 * so detect it and create a different expression type.
 *
 * Note too that I'm using *right* recursion to implement the {...}
 * part of the rule. This is normally bad, but expression lists aren't
 * normally that long, and the while loop going through the formed
 * list fixes up the associations.
 */
simple_expression
  : '-' simple_expression_2
      { $$ = new ExpUMinus($2); }
  | '+' simple_expression_2
      { $$ = $2; }
  | simple_expression_2
      { $$ = $1; }
  ;

simple_expression_2
  : term
      { $$ = $1; }
  | term simple_expression_terms
      { Expression*tmp = $1;
	list<struct adding_term>*lst = $2;
	while (! lst->empty()) {
	      struct adding_term item = lst->front();
	      lst->pop_front();
	      if (item.op == ExpArithmetic::xCONCAT)
		    tmp = new ExpConcat(tmp, item.term);
	      else
		    tmp = new ExpArithmetic(item.op, tmp, item.term);
	}
	delete lst;
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  ;

simple_expression_terms
  : adding_operator term
      { struct adding_term item;
	item.op = $1;
	item.term = $2;
	list<adding_term>*tmp = new list<adding_term>;
	tmp->push_back(item);
	$$ = tmp;
      }
  | simple_expression_terms adding_operator term
      { list<adding_term>*tmp = $1;
	struct adding_term item;
	item.op = $2;
	item.term = $3;
	tmp->push_back(item);
	$$ = tmp;
      }
  ;

signal_assignment
  : name LEQ waveform ';'
      { SignalSeqAssignment*tmp = new SignalSeqAssignment($1, $3);
	FILE_NAME(tmp, @1);
	delete $3;
	$$ = tmp;
      }
  | name LEQ waveform K_when expression K_else waveform ';'
      { SignalSeqAssignment*tmp = new SignalSeqAssignment($1, $3);
	FILE_NAME(tmp, @1);
	sorrymsg(@4, "Conditional signal assignment not supported.\n");
	$$ = tmp;
      }
  ;

signal_assignment_statement
  : signal_assignment
  | IDENTIFIER ':' signal_assignment
      { delete[] $1;
	$$ = $3;
      }

subprogram_body_start
  : subprogram_specification K_is
      { assert(!active_sub);
        active_sub = $1;
        $$ = $1; }
  ;

  /* This is a function/task body. This may have a matching subprogram
     declaration, and if so it will be in the active scope. */

subprogram_body /* IEEE 1076-2008 P4.3 */
  : subprogram_body_start subprogram_declarative_part
    K_begin subprogram_statement_part K_end
    subprogram_kind_opt identifier_opt ';'
      { SubprogramHeader*prog = $1;
	SubprogramHeader*tmp = active_scope->recall_subprogram(prog);
	if (tmp) {
	      delete prog;
	      prog = tmp;
	}

	SubprogramBody*body = new SubprogramBody();
	body->transfer_from(*active_scope, ScopeBase::VARIABLES);
	body->set_statements($4);

	prog->set_body(body);
	active_scope->bind_subprogram(prog->name(), prog);
	active_sub = NULL;
      }

  | subprogram_body_start
    subprogram_declarative_part
    K_begin error K_end
    subprogram_kind_opt identifier_opt ';'
      { errormsg(@2, "Syntax errors in subprogram body.\n");
	yyerrok;
	active_sub = NULL;
	if ($1) delete $1;
	if ($7) delete[]$7;
      }
  ;

subprogram_declaration
  : subprogram_specification ';'
      { if ($1) active_scope->bind_subprogram($1->name(), $1); }
  ;

subprogram_declarative_item /* IEEE 1079-2008 P4.3 */
  : variable_declaration
  | file_declaration
  ;

subprogram_declarative_item_list
  : subprogram_declarative_item_list subprogram_declarative_item
  | subprogram_declarative_item
  ;

subprogram_declarative_part /* IEEE 1076-2008 P4.3 */
  : subprogram_declarative_item_list
  |
  ;

subprogram_kind /* IEEE 1076-2008 P4.3 */
  : K_function
  | K_procedure
  ;

subprogram_kind_opt : subprogram_kind | ;

subprogram_specification
  : function_specification { $$ = $1; }
  | procedure_specification { $$ = $1; }
  ;

  /* This is an implementation of the rule:
     subprogram_statement_part ::= { sequential_statement }
     where the sequence_of_statements rule is a list of
     sequential_statement. */
subprogram_statement_part
  : sequence_of_statements { $$ = $1; }
  ;

subtype_declaration
  : K_subtype IDENTIFIER K_is subtype_indication ';'
      { perm_string name = lex_strings.make($2);
	if ($4 == 0) {
	      errormsg(@1, "Failed to declare type name %s.\n", name.str());
	} else {
	      VTypeDef*tmp;
	      map<perm_string,VTypeDef*>::iterator cur = active_scope->incomplete_types.find(name);
	      if (cur == active_scope->incomplete_types.end()) {
		    tmp = new VSubTypeDef(name, $4);
		    active_scope->bind_name(name, tmp);
	      } else {
		    tmp = cur->second;
		    tmp->set_definition($4);
		    active_scope->incomplete_types.erase(cur);
	      }
	}
	delete[]$2;
      }
  ;

subtype_indication
  : IDENTIFIER
      { const VType*tmp = parse_type_by_name(lex_strings.make($1));
	if (tmp == 0) {
	      errormsg(@1, "Can't find type name `%s'\n", $1);
	      tmp = new VTypeERROR;
	}
	delete[]$1;
	$$ = tmp;
      }
  | IDENTIFIER index_constraint
      { const VType*tmp = calculate_subtype_array(@1, $1, active_scope, $2);
	if (tmp == 0) {
	      errormsg(@1, "Unable to calculate bounds for array of %s.\n", $1);
	}
	delete[]$1;
	delete  $2;
	$$ = tmp;
      }
  | IDENTIFIER K_range simple_expression direction simple_expression
      { const VType*tmp = calculate_subtype_range(@1, $1, active_scope, $3, $4, $5);
	if (tmp == 0) {
	      errormsg(@1, "Unable to calculate bounds for range of %s.\n", $1);
	}
	delete[]$1;
	$$ = tmp;
      }
  ;

suffix
  : IDENTIFIER
      { $$ = $1; }
  | CHARACTER_LITERAL
      { $$ = $1; }
  | K_all
      { //do not have now better idea than using char constant
	$$ = strdup("all");
      }
  ;

term
  : factor
      { $$ = $1; }
  | factor '*' factor
      { ExpArithmetic*tmp = new ExpArithmetic(ExpArithmetic::MULT, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | factor '/' factor
      { ExpArithmetic*tmp = new ExpArithmetic(ExpArithmetic::DIV, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | factor K_mod factor
      { ExpArithmetic*tmp = new ExpArithmetic(ExpArithmetic::MOD, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  | factor K_rem factor
      { ExpArithmetic*tmp = new ExpArithmetic(ExpArithmetic::REM, $1, $3);
	FILE_NAME(tmp, @2);
	$$ = tmp;
      }
  ;

type_declaration
  : K_type IDENTIFIER K_is type_definition ';'
      { perm_string name = lex_strings.make($2);
	if ($4 == 0) {
	      errormsg(@1, "Failed to declare type name %s.\n", name.str());
	} else {
	      VTypeDef*tmp;
	      map<perm_string,VTypeDef*>::iterator cur = active_scope->incomplete_types.find(name);
	      if (cur == active_scope->incomplete_types.end()) {
		    tmp = new VTypeDef(name, $4);
		    active_scope->bind_name(name, tmp);
	      } else {
		    tmp = cur->second;
		    tmp->set_definition($4);
		    active_scope->incomplete_types.erase(cur);
	      }
	}
	delete[]$2;
      }
  | K_type IDENTIFIER ';'
      { perm_string name = lex_strings.make($2);
	VTypeDef*tmp = new VTypeDef(name);
	active_scope->incomplete_types[name] = tmp;
	active_scope->bind_name(name, tmp);
	delete[]$2;
      }
  | K_type IDENTIFIER K_is error ';'
      { errormsg(@4, "Error in type definition for %s\n", $2);
	yyerrok;
	delete[]$2;
      }
  | K_type error ';'
      { errormsg(@1, "Error in type definition\n");
	yyerrok;
      }
  ;

type_definition
  : '(' enumeration_literal_list ')'
      { VTypeEnum*tmp = new VTypeEnum($2);
	active_scope->use_enum(tmp);
	delete $2;
	$$ = tmp;
      }
  | composite_type_definition
      { $$ = $1; }

  ;

use_clause
  : K_use selected_names ';'
     { $$ = $2; }
  | K_use error ';'
     { errormsg(@1, "Syntax error in use clause.\n"); yyerrok; }
  ;

use_clause_lib
  : K_use selected_names_lib ';'
  | K_use error ';'
     { errormsg(@1, "Syntax error in use clause.\n"); yyerrok; }
  ;

use_clauses_lib
  : use_clauses_lib use_clause_lib
  | use_clause_lib
  ;

use_clauses_opt
  : use_clauses_lib
  |
  ;

variable_assignment_statement /* IEEE 1076-2008 P10.6.1 */
  : variable_assignment
  | IDENTIFIER ':' variable_assignment
      { delete[] $1;
	$$ = $3;
      }

variable_assignment
  : name VASSIGN expression ';'
      { VariableSeqAssignment*tmp = new VariableSeqAssignment($1, $3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | name VASSIGN error ';'
      { errormsg(@3, "Syntax error in r-value expression of assignment.\n");
	yyerrok;
	delete $1;
	$$ = 0;
      }
  | error VASSIGN expression ';'
      { errormsg(@1, "Syntax error in l-value expression of assignment.\n");
	yyerrok;
	delete $3;
	$$ = 0;
      }
  ;

variable_declaration /* IEEE 1076-2008 P6.4.2.4 */
  : K_shared_opt K_variable identifier_list ':' subtype_indication
    variable_declaration_assign_opt ';'
      { /* Save the signal declaration in the block_signals map. */
	for (std::list<perm_string>::iterator cur = $3->begin()
		   ; cur != $3->end() ; ++cur) {
	      Variable*sig = new Variable(*cur, $5, $6);
	      FILE_NAME(sig, @2);
	      active_scope->bind_name(*cur, sig);
	}
	delete $3;
      }
  | K_shared_opt K_variable error ';'
      { errormsg(@2, "Syntax error in variable declaration.\n");
	yyerrok;
      }
  ;

variable_declaration_assign_opt
  : VASSIGN expression { $$ = $2; }
  |                    { $$ = 0;  }
  ;

wait_statement
  : K_wait K_for expression ';'
      { WaitForStmt*tmp = new WaitForStmt($3);
	FILE_NAME(tmp, @1);
	$$ = tmp;
      }
  | K_wait K_on expression ';'
      { WaitStmt*tmp = new WaitStmt(WaitStmt::ON, $3);
        FILE_NAME(tmp, @1);
        $$ = tmp;
      }
  | K_wait K_until expression ';'
      { WaitStmt*tmp = new WaitStmt(WaitStmt::UNTIL, $3);
        FILE_NAME(tmp, @1);
        $$ = tmp;
      }
  | K_wait ';'
      { WaitStmt*tmp = new WaitStmt(WaitStmt::FINAL, NULL);
        FILE_NAME(tmp, @1);
        $$ = tmp;
      }
  ;

waveform
  : waveform_elements
      { $$ = $1; }
  | K_unaffected
      { $$ = 0; }
  ;

waveform_elements
  : waveform_elements ',' waveform_element
      { std::list<Expression*>*tmp = $1;
	tmp->push_back($3);
	$$ = tmp;
      }
  | waveform_element
      { std::list<Expression*>*tmp = new std::list<Expression*>;
	tmp->push_back($1);
	$$ = tmp;
      }
  ;

waveform_element
  : expression
      { $$ = $1; }
  | expression K_after expression
      { ExpDelay*tmp = new ExpDelay($1, $3);
        FILE_NAME(tmp, @1);
        $$ = tmp; }
  | K_null
      { $$ = 0; }
  ;

  /* Some keywords are optional in some contexts. In all such cases, a
     similar rule is used, as described here. */
K_architecture_opt : K_architecture | ;
K_configuration_opt: K_configuration| ;
K_entity_opt       : K_entity       | ;
K_is_opt           : K_is           | ;
K_package_opt      : K_package      | ;
K_postponed_opt    : K_postponed    | ;
K_shared_opt       : K_shared       | ;
%%

static void yyerror(YYLTYPE*loc, yyscan_t, const char*, bool, const char*msg)
{
      fprintf(stderr, "%s:%u: %s\n", loc->text, loc->first_line, msg);
      parse_errors += 1;
}

void errormsg(const YYLTYPE&loc, const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);

      fprintf(stderr, "%s:%u: error: ", loc.text, loc.first_line);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      parse_errors += 1;
}

void sorrymsg(const YYLTYPE&loc, const char*fmt, ...)
{
      va_list ap;
      va_start(ap, fmt);

      fprintf(stderr, "%s:%u: sorry: ", loc.text, loc.first_line);
      vfprintf(stderr, fmt, ap);
      va_end(ap);
      parse_sorrys += 1;
}


/*
 * The reset_lexor function takes the fd and makes it the input file
 * for the lexor. The path argument is used in lexor/parser error messages.
 */
extern yyscan_t prepare_lexor(FILE*fd);
extern void destroy_lexor(yyscan_t scanner);

int parse_source_file(const char*file_path, perm_string parse_library_name)
{
      FILE*fd = fopen(file_path, "r");
      if (fd == 0) {
	    perror(file_path);
	    return -1;
      }

      yyscan_t scanner = prepare_lexor(fd);
      int rc = yyparse(scanner, file_path, parse_library_name);
      fclose(fd);
      destroy_lexor(scanner);

      return rc;
}
