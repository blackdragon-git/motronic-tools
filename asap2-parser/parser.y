/* Copyright (C) Josef Schmeißer 2011
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

%{
	#include <cstdio>
//	#include <stack>
	#include "stack.hpp"
	#include "node.h"
	#include "util.h"

	NProject* projectBlock; /* the top level root node of our final AST */
	extern int yylineno; // TO/DO
	extern int yylex();
	void yyerror(const char *s) { printf("Error at line %d: %s\n", yylineno, s); }

ext::stack<Node *> nodes;
%}

/* Represents the many different ways we can access our data */
%union {
	Node *node;
	NBlock *block;
	NExpression *expr;
	NStatement *stmt;
	NIdentifier *ident;
//	NVariableDeclaration *var_decl;
//	std::vector<NVariableDeclaration*> *varvec;
NCharacteristic *characteristic;
NFormat *format;
NAxis *axis;
NNumeric *numeric;
NAddress *address;
NVariable *var_def;
NProject* project;
NHeader* header;
NModule* module;
NRecordLayout::FncValues* fncValues;

	std::vector<NExpression*> *exprvec;
	std::vector<NStatement*> *stmtvec;

	std::string *string;
	int token;

	int value;
	bool flag;
}
/* Define our terminal symbols (tokens). This should
   match our tokens.l lex file. We also define the node type
   they represent.
 */
%token <string> TADDRESS TSTRING TIDENTIFIER TDOUBLE TINTEGER
%token <token> TLBRACE TRBRACE
%token <token> TUWORD TSWORD TUBYTE TSBYTE TULONG TSLONG TFLOAT32
%token <token> TABSOLUTE TAXIS_DESCR TAXIS_PTS TCHARACTERISTIC TCOMPU_METHOD TCOM_AXIS TCURVE TDEF_CHARACTERISTIC TDEPOSIT TFORMAT TFUNCTION TSTD_AXIS  TMAP TMODULE TPROJECT TVALUE TVAL_BLK TMEASUREMENT TREF_CHARACTERISTIC TIN_MEASUREMENT TOUT_MEASUREMENT TLOC_MEASUREMENT TSUB_FUNCTION TMOD_COMMON TMOD_PAR TBYTE_ORDER TMSB_LAST TALIGNMENT_BYTE TALIGNMENT_WORD TALIGNMENT_LONG TMEMORY_SEGMENT TCODE TEPROM TEXTERN TINTERN TSYSTEM_CONSTANT TECU_ADDRESS TBIT_MASK TAXIS_PTS_REF TFIX_AXIS TFIX_AXIS_PAR TB_TRUE TARRAY_SIZE TREAD_ONLY TNUMBER TRAT_FUNC TCOEFFS TCOMPU_TAB TTAB_INTP TASCII TTAB_VERB TCOMPU_TAB_REF TCOMPU_VTAB TASAP2_VERSION THEADER TVERSION TPROJECT_NO

// record_layout tokens:
%token <token> TRECORD_LAYOUT TNO_AXIS_PTS_X TNO_AXIS_PTS_Y TAXIS_PTS_X TAXIS_PTS_Y TINDEX_INCR TFNC_VALUES TCOLUMN_DIR TDIRECT

/* Define the type of node our nonterminal symbols represent.
   The types refer to the %union declaration above. Ex: when
   we call an ident (defined by union type ident) we are really
   calling an (NIdentifier*). It makes the compiler happy.
 */
%type <stmt> stmt characteristic axis_pts measurement function record_layout compu_method compu_tab compu_vtab memory_segment mod_par system_constant mod_common
%type <ident> ident
%type <block>  stmts //project
%type <format> format format_optional

%type <expr> string expr //numeric
%type <var_def> var_def
%type <project> project
%type <module> module
%type <header> header

%type <token> type
%type <axis> axis_desc std_axis com_axis fix_axis
%type <numeric> numeric
%type <address> address

%type <value> number_tag
%type <flag> access
%type <exprvec> ident_list numeric_list def_characteristic ref_characteristic in_measurement out_measurement loc_measurement sub_function
%type <stmtvec> system_constant_list var_defs

%type <string> bit_mask

%start project

%%

// main block
project :	TASAP2_VERSION numeric numeric
		TLBRACE TPROJECT ident string
			header
			module
		TRBRACE TPROJECT
		{ projectBlock = new NProject(*$8, *$9); YYACCEPT; }
	;

header :	TLBRACE THEADER TSTRING
			TVERSION TSTRING
			TPROJECT_NO ident
		TRBRACE THEADER
		{
			$$ = new NHeader(*$3, *$5, *$7);
		}
	;

module :	TLBRACE TMODULE ident string
			mod_par
			mod_common
			stmts
		TRBRACE TMODULE
		{
			$$ = new NModule(*$7);
		}
	;

stmts : stmt { $$ = new NBlock(); $$->statements.push_back($<stmt>1); }
	| stmts stmt { $1->statements.push_back($<stmt>2); }
	;

stmt : characteristic
	| axis_pts
	| measurement
	| function
	| record_layout
	| compu_method
	| compu_tab
	| compu_vtab
	;

expr :	  ident { $<ident>$ = $1; }
	| numeric { $<expr>$ = $1; }
	| string
	;

ident_list : /* empty */ { $$ = new ExpressionList(); } | ident_list ident { $1->push_back($2); }
	;

ident : TIDENTIFIER { $$ = new NIdentifier(*$1); }
	| TVERSION { $$ = new NIdentifier("VERSION"); } // workaround
	;

system_constant_list : /* empty */ { $$ = new StatementList(); } | system_constant_list system_constant { $1->push_back($2); }
	;

system_constant : TSYSTEM_CONSTANT TSTRING TSTRING
	{
		NExpression* expr = new NInteger(atol($3->c_str())); // should always be an integer
		NIdentifier* ident = new NIdentifier(*$2);
		$$ = new NConstant(ident, *expr);
	}
	;

var_defs :  /* empty */ { $$ = new StatementList(); } | var_defs var_def { $1->push_back($2); }
	;

var_def : ident expr { $$ = new NVariable($1); $$->assignmentExpr = $2; }
	;

numeric_list : /* empty */ { $$ = new ExpressionList(); } | numeric_list numeric { $1->push_back($2); }
	;

numeric : TINTEGER { $$ = new NInteger(atol($1->c_str())); }
	| TDOUBLE { $$ = new NDouble(atof($1->c_str())); }
	| address { $$ = $1 }
	;

address: TADDRESS
	{
		std::string addr = getAddressSubstr(*$1); // remove leading '"0x' and rear '"' characters
		char* p;
		unsigned long n = strtoul(addr.c_str(), &p, 16); // addresses are hexadecimal
		if (*p != 0) {  
			printf("Invalid address at line %d\n", yylineno);
			YYERROR;
		}

		$$ = new NAddress(n);
	}
	;

string : TSTRING { $$ = new NStringLiteral(*$1); }
	;

type : TUWORD | TSWORD | TUBYTE | TSBYTE | TULONG | TSLONG | TFLOAT32
	;

access : /* empty */ { $$ = true } | TREAD_ONLY { printf("\tREAD_ONLY mark\n"); $$ = false; }
	;

number_tag : /* empty */ { $$ = 0 } | TNUMBER TINTEGER { $$ = atoi($2->c_str()); }
	;

memory_segment : TLBRACE TMEMORY_SEGMENT
			ident
			TSTRING
			ident_list
			numeric_list
		TRBRACE TMEMORY_SEGMENT
		{
			printf("\tmemory-segment\n");
			$$ = NULL; // TODO
		}
	;

mod_par :	TLBRACE TMOD_PAR TSTRING
			var_defs // TODO
			memory_segment // TODO list
			memory_segment
			memory_segment
			memory_segment
			memory_segment
			memory_segment
			system_constant_list
		TRBRACE TMOD_PAR
		{
			printf("\tmod_par\n");
			$$ = NULL;
		}
	;

mod_common :	TLBRACE TMOD_COMMON TSTRING
			TBYTE_ORDER TMSB_LAST // TODO
			TALIGNMENT_BYTE TINTEGER
			TALIGNMENT_WORD TINTEGER
			TALIGNMENT_LONG TINTEGER
		TRBRACE TMOD_COMMON
		{
			printf("\tmod_common\n");
			$$ = NULL;
		}
	;

characteristic : TLBRACE TCHARACTERISTIC // com-axis
			ident
			TSTRING // description std::string
			TMAP
			address //TADDRESS
			ident
			TDOUBLE // scale std::string to double
			ident
			TDOUBLE // min std::string to double
			TDOUBLE // max std::string to double
			format
			access
			axis_desc //com_axis //axis_desc
			axis_desc //com_axis //axis_desc
		TRBRACE TCHARACTERISTIC
		{
			printf ("\tcharacteristic-map: %s\n", $3->name.c_str());

			$$ = /*new NMap<NComAxis>*/ createMap($3 /* name */,
					*$4 /* description */,
					*$6 /* address */,
					*$7 /* recordLayout */,
					atof($8->c_str()) /* scale */,
					*$9 /* type */,
					atof($10->c_str()) /* min */,
					atof($11->c_str()) /* max */,
					*$12 /* format */,
					*$14 /* axis_1 */,
					*$15 /* axis_2 */);
			if ($$ == NULL) { YYERROR; }
		}
	|
		TLBRACE TCHARACTERISTIC
			ident
			TSTRING // description std::string
			TCURVE
			address //TADDRESS
			ident
			TDOUBLE // scale std::string to double
			ident
			TDOUBLE // min std::string to double
			TDOUBLE // max std::string to double
			format
			access
			axis_desc
		TRBRACE TCHARACTERISTIC
		{
			printf ("\tcharacteristic-curve: %s\n", $3->name.c_str());

			$$ = new NCurve($3 /* name */,
					*$4 /* description */,
					*$6 /* address */,
					*$7 /* recordLayout */,
					atof($8->c_str()) /* scale */,
					*$9 /* type */,
					atof($10->c_str()) /* min */,
					atof($11->c_str()) /* max */,
					*$12 /* format */,
					*$14 /* axis_1 */);
		}
	|
		TLBRACE TCHARACTERISTIC
			ident
			TSTRING // description std::string
			TVALUE
			address //TADDRESS
			ident
			TDOUBLE // scale std::string to double
			ident
			TDOUBLE // min std::string to double
			TDOUBLE // max std::string to double
			format
			access
		TRBRACE TCHARACTERISTIC
		{
			printf ("\tcharacteristic-value: %s %s\n", $3->name.c_str(), $4->c_str());

			$$ = new NValue($3 /* name */,
					*$4 /* description */,
					*$6 /* address */,
					*$7 /* recordLayout */,
					atof($8->c_str()) /* scale */,
					*$9 /* type */,
					atof($10->c_str()) /* min */,
					atof($11->c_str()) /* max */,
					*$12 /* format */);
		}
	|
		TLBRACE TCHARACTERISTIC
			ident
			TSTRING // description std::string
			TVAL_BLK
			address //TADDRESS
			ident
			TDOUBLE // scale std::string to double
			ident
			TDOUBLE // min std::string to double
			TDOUBLE // max std::string to double
			format
			access
			number_tag
		TRBRACE TCHARACTERISTIC
		{
			printf ("\tcharacteristic-valblk: %s number: %d\n", $3->name.c_str(), $14);

			$$ = new NValBlk($3 /* name */,
					*$4 /* description */,
					*$6 /* address */,
					*$7 /* recordLayout */,
					atof($8->c_str()) /* scale */,
					*$9 /* type */,
					atof($10->c_str()) /* min */,
					atof($11->c_str()) /* max */,
					*$12 /* format */,
					$14);	// number
		}
	|
		TLBRACE TCHARACTERISTIC
			ident
			TSTRING // description std::string
			TASCII
			address //TADDRESS
			ident
			TDOUBLE // scale std::string to double
			ident
			TDOUBLE // min std::string to double
			TDOUBLE // max std::string to double
			format_optional
			access
			number_tag
		TRBRACE TCHARACTERISTIC
		{
			printf ("\tcharacteristic-ascii: %s number: %d\n", $3->name.c_str(), $14);

			$$ = new NCharacteristicText($3 /* name */,
					*$4 /* description */,
					*$6 /* address */,
					*$7 /* recordLayout */,
					atof($8->c_str()) /* scale */,
					*$9 /* type */,
					atof($10->c_str()) /* min */,
					atof($11->c_str()) /* max */,
					*$12 /* format */,
					$14);	// number
		}
	; // characteristic

measurement :	TLBRACE TMEASUREMENT
			ident
			TSTRING // description std::string
			type		// datatype
			TB_TRUE
			TINTEGER // int1 std::string to int
			TINTEGER // int2 std::string to int
			numeric		// min; double or int
			numeric		// max; double or int
			bit_mask	// bitMask std::string*
			format
			TECU_ADDRESS address //TADDRESS
		TRBRACE TMEASUREMENT
		{
			printf ("\tmeasurement-bit: %s\n", $3->name.c_str());

			$$ = new NMeasurementBit($3,	// name
						*$4,	// description
						$5,	// dataType
						atoi($7->c_str()), // int1
						atoi($8->c_str()), // int2
						*$9,	// min
						*$10,	// max
						*$12,	// format
						*$14,	// address
						*$11);	// bitMask
		}
	| // without bitmask
		TLBRACE TMEASUREMENT
			ident
			TSTRING // description std::string
			type		// datatype
			ident		// samples: dez, t10msxs_ub_b2p55
			TINTEGER // int1 std::string to int
			TINTEGER // int2 std::string to int
			numeric		// min; double or int
			numeric		// max; double or int
			format
			TECU_ADDRESS address //TADDRESS
		TRBRACE TMEASUREMENT
		{
			printf ("\tmeasurement-value: %s\n", $3->name.c_str());

			$$ = new NMeasurementValue($3,	// name
						*$4,	// description
						$5,	// dataType
						atoi($7->c_str()), // int1
						atoi($8->c_str()), // int2
						*$9,	// min
						*$10,	// max
						*$11,	// format
						*$13,	// address
						*$6);	// type
		}
	| // with bitmask
		TLBRACE TMEASUREMENT
			ident
			TSTRING // description std::string
			type		// datatype
			ident		// samples: dez, t10msxs_ub_b2p55
			TINTEGER // int1 std::string to int
			TINTEGER // int2 std::string to int
			numeric		// min; double or int
			numeric		// max; double or int
			bit_mask // std::string*
			format
			TECU_ADDRESS address //TADDRESS
		TRBRACE TMEASUREMENT
		{
			printf ("\tmeasurement-value: %s\n", $3->name.c_str());

			$$ = new NMeasurementValue($3,	// name
						*$4,	// description
						$5,	// dataType
						atoi($7->c_str()), // int1
						atoi($8->c_str()), // int2
						*$9,	// min
						*$10,	// max
						*$12,	// format
						*$14,	// address
						*$6);	// type
		}
	|
		TLBRACE TMEASUREMENT
			ident
			TSTRING // description std::string
			type		// datatype
			ident		// samples: dez, t10msxs_ub_b2p55
			TINTEGER // int1 std::string to int
			TINTEGER // int2 std::string to int
			numeric		// min; double or int
			numeric		// max; double or int
			format
			TARRAY_SIZE TINTEGER // arraySize std::string to int
			TECU_ADDRESS address //TADDRESS
		TRBRACE TMEASUREMENT
		{
			printf ("\tmeasurement-array: %s\n", $3->name.c_str());

			$$ = new NMeasurementArray($3,	// name
						*$4,	// description
						$5,	// dataType
						atoi($7->c_str()), // int1
						atoi($8->c_str()), // int2
						*$9,	// min
						*$10,	// max
						*$11,	// format
						*$15,	// address
						*$6,	// type
						atoi($13->c_str())); // arraySize
		}
	; // measurement

axis_desc : std_axis { $$ = $1; }
	| com_axis { $$ = $1; }
	| fix_axis { $$ = $1; }
	;  // axis_desc

std_axis :	TLBRACE TAXIS_DESCR
			TSTD_AXIS
			ident
			ident // compuMethod
			TINTEGER
			TDOUBLE
			TDOUBLE
			format
			deposit
		TRBRACE TAXIS_DESCR
		{
			printf ("\tstd-axis\n");
			$$ = new NStdAxis(*$4, *$5, atol($6->c_str()), atof($7->c_str()), atof($8->c_str()), *$9);
		}
	;

com_axis :	TLBRACE TAXIS_DESCR
			TCOM_AXIS
			ident
			ident // compuMethod
			TINTEGER
			TDOUBLE
			TDOUBLE
			TAXIS_PTS_REF ident
		TRBRACE TAXIS_DESCR
		{
			printf ("\tcom-axis\n");
			$$ = new NComAxis(*$4, *$5, atol($6->c_str()), atof($7->c_str()), atof($8->c_str()), *$10);
		}
	;

fix_axis :	TLBRACE TAXIS_DESCR
			TFIX_AXIS
			ident
			ident // compuMethod
			TINTEGER
			TDOUBLE
			TDOUBLE
			format
			TFIX_AXIS_PAR TINTEGER TINTEGER TINTEGER
		TRBRACE TAXIS_DESCR
		{
			printf ("\tfix-axis\n");
			$$ = new NFixAxis(*$4, *$5, atol($6->c_str()), atof($7->c_str()), atof($8->c_str()), *$9);
		}
	;

axis_pts :	TLBRACE TAXIS_PTS
			ident
			TSTRING // std::string description
			address //TADDRESS
			ident
			ident
			TDOUBLE // std::string to double scale
			ident
			TINTEGER // std::string to int size
			TDOUBLE // std::string to double min
			TDOUBLE // std::string to double max
			format
			deposit
		TRBRACE TAXIS_PTS
		{
			printf("\taxis-pts: %s %s\n", $3->name.c_str(), $4->c_str());
			$$ = new NAxisPts($3,	// name
					*$4,	// description
					*$5,	// address
					*$6,	// unit
					*$7,	// ident
					atof($8->c_str()),	// scale
					*$9,	//type
					atoi($10->c_str()),	// size
					atof($11->c_str()),	// min
					atof($12->c_str()),	// max
					*$13);	// format
		}
	; // axis_pts

record_layout : TLBRACE TRECORD_LAYOUT ident
			fnc_values
		TRBRACE TRECORD_LAYOUT
		{
			$$ = NRecordLayout::createRecordLayout($3, // name
				$<fncValues>4); // fnc_values
			if ($$ == NULL) { YYERROR; }
		}
	| // for a curve
		TLBRACE TRECORD_LAYOUT ident
			TNO_AXIS_PTS_X TINTEGER type
			TAXIS_PTS_X TINTEGER type TINDEX_INCR TDIRECT
			fnc_values
		TRBRACE TRECORD_LAYOUT
		{
			$$ = NRecordLayout::createRecordLayout($3, // name
				$6, // no-type X
				$9, // val-type X
				0, // TODO flags
				$<fncValues>12); // fnc_values

			// $5, $8 => these integers are not used
			if ($$ == NULL) { YYERROR; }
		}
	| // for a map
		TLBRACE TRECORD_LAYOUT ident
			TNO_AXIS_PTS_X TINTEGER type
			TNO_AXIS_PTS_Y TINTEGER type
			TAXIS_PTS_X TINTEGER type TINDEX_INCR TDIRECT
			TAXIS_PTS_Y TINTEGER type TINDEX_INCR TDIRECT
			fnc_values
		TRBRACE TRECORD_LAYOUT
		{
			$$ = NRecordLayout::createRecordLayout($3, // name
				$6, // no-type X
				$12, // val-type X
				0, // TODO flags
				$9, // no-type Y
				$17, // val-type Y
				0, // TODO flags
				$<fncValues>20); // fnc_values

			// $5, $8, $11, $16 => these integers are not used
			if ($$ == NULL) { YYERROR; }
		}
	; // record_layout

fnc_values : /* empty */ { $<fncValues>$ = NULL; }
	| TFNC_VALUES TINTEGER type TCOLUMN_DIR TDIRECT
	{
		$<fncValues>$ = new NRecordLayout::FncValues($3, // type
			0); // TODO flags
	}
	;

compu_method :	TLBRACE TCOMPU_METHOD
			ident
			TSTRING
			TRAT_FUNC
			TSTRING
			TSTRING
			TCOEFFS numeric numeric numeric numeric numeric numeric
		TRBRACE TCOMPU_METHOD
		{
			printf("\tcompu_method: %s %s\n", $3->name.c_str(), $4->c_str());

			NFormat* format = new NFormat(*$6);
			$$ = new NCompuMethod($3,	// name
						*$4,	// description
						*format, // format
						*$7,	// unit
						*$9,	// number1
						*$10,	// number2
						*$11,	// number3
						*$12,	// number4
						*$13,	// number5
						*$14);	// number6
		}
	| // boolean
		TLBRACE TCOMPU_METHOD
			TB_TRUE
			TSTRING
			TTAB_VERB
			TSTRING
			TSTRING
			TCOMPU_TAB_REF TB_TRUE
		TRBRACE TCOMPU_METHOD
		{
			printf("\tcompu_method-boolean: %s\n", $4->c_str());
$$ = NULL;
/* // TODO
			NFormat* format = new NFormat(*$6);
			$$ = new NCompuMethod(*$3,	// name
						*$4,	// description
						*format, // format
						*$7,	// unit
						*$9,	// number1
						*$10,	// number2
						*$11,	// number3
						*$12,	// number4
						*$13,	// number5
						*$14);	// number6
*/
		}
	| // tab_intp
		TLBRACE TCOMPU_METHOD
			ident
			TSTRING
			TTAB_INTP
			TSTRING
			TSTRING
			TCOMPU_TAB_REF ident
		TRBRACE TCOMPU_METHOD
		{
			printf("\tcompu_method-tab_intp: %s\n", $3->name.c_str());
$$ = NULL;
/* // TODO
			NFormat* format = new NFormat(*$6);
			$$ = new NCompuMethod(*$3,	// name
						*$4,	// description
						*format, // format
						*$7,	// unit
						*$9,	// number1
						*$10,	// number2
						*$11,	// number3
						*$12,	// number4
						*$13,	// number5
						*$14);	// number6
*/
		}
	; // compu_method

compu_tab :	TLBRACE TCOMPU_TAB
			ident
			TSTRING
			TTAB_INTP
			TINTEGER
			compu_tab_list
		TRBRACE TCOMPU_TAB
		{
			$$ = NULL; // TODO
		}
	; // compu_tab

compu_tab_list	: compu_tab_item { }
		| compu_tab_list compu_tab_item { }
	;

compu_tab_item : TINTEGER numeric { delete $2; } // TODO implement
	;

compu_vtab :	TLBRACE TCOMPU_VTAB
			TB_TRUE
			TSTRING
			TTAB_VERB
			TINTEGER
			TINTEGER TSTRING
			TINTEGER TSTRING
		TRBRACE TCOMPU_VTAB
		{
			$$ = NULL;//new NStatement(); // TODO
		}
	;

function :	TLBRACE TFUNCTION
			ident
			TSTRING
			def_characteristic
			ref_characteristic
			in_measurement
			out_measurement
			loc_measurement
			sub_function
		TRBRACE TFUNCTION
		{
			printf("\tfunction: %s %s\n", $3->name.c_str(), $4->c_str());
			$$ = new NFunction($3, *$4, *$5, *$6, *$7, *$8, *$9, *$10);
		}
	; // function


def_characteristic :	TLBRACE TDEF_CHARACTERISTIC
				ident_list
			TRBRACE TDEF_CHARACTERISTIC
			{ $$ = $3; }
		;

ref_characteristic :	TLBRACE TREF_CHARACTERISTIC
				ident_list
			TRBRACE TREF_CHARACTERISTIC
			{ $$ = $3; }
		;

in_measurement :	TLBRACE TIN_MEASUREMENT
				ident_list
			TRBRACE TIN_MEASUREMENT
			{ $$ = $3; }
		;

out_measurement :	TLBRACE TOUT_MEASUREMENT
				ident_list
			TRBRACE TOUT_MEASUREMENT
			{ $$ = $3; }
		;

loc_measurement :	TLBRACE TLOC_MEASUREMENT
				ident_list
			TRBRACE TLOC_MEASUREMENT
			{ $$ = $3; }
		;

sub_function :	TLBRACE TSUB_FUNCTION
			ident_list
		TRBRACE TSUB_FUNCTION
		{ $$ = $3; }
	;

format_optional : /* empty */ { $$ = NULL; } | format { $$ = $1; }
	;

format : TFORMAT TSTRING { printf("\tformat: %s\n", $2->c_str()); $$ = new NFormat(*$2); }
	;

//bit_mask_optional : /* empty / { $$ = NULL; }*/ | bit_mask { $$ = $1; }
//	;

bit_mask : TBIT_MASK TADDRESS { printf("\tbit-mask: %s\n", $2->c_str()); $$ = $2; }
	;

deposit : TDEPOSIT TABSOLUTE { printf("\tdeposite: absolute\n"); } // TODO
	;

%%

