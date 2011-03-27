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
#include <string>
#include <vector>
//#include <iostream>
#include "node.h"
#include "parser.hpp"

std::vector<std::string*> value_tokens;

#define SAVE_TOKEN yylval.string = new std::string(yytext, yyleng);    \
    value_tokens.push_back(yylval.string);

#define SAVE_STRING yylval.string = new std::string(yytext + 1, yyleng - 2);    \
//std::cout << *yylval.string << std::endl; \
    value_tokens.push_back(yylval.string);

#define TOKEN(t) (yylval.token = t)

extern "C" int yywrap() { }
extern void yyerror (const char *s);
%}

%x IN_COMMENT IN_A2ML IN_IF_DATA

%%

[ \t\n]					;

UWORD					return TOKEN(TUWORD);
SWORD					return TOKEN(TSWORD);
UBYTE					return TOKEN(TUBYTE);
SBYTE					return TOKEN(TSBYTE);
ULONG					return TOKEN(TULONG);
SLONG					return TOKEN(TSLONG);
FLOAT32_IEEE				return TOKEN(TFLOAT32);

"ASAP2_VERSION"				return TOKEN(TASAP2_VERSION);
"ABSOLUTE"				return TOKEN(TABSOLUTE);
"AXIS_DESCR"				return TOKEN(TAXIS_DESCR);
"AXIS_PTS"				return TOKEN(TAXIS_PTS);
"CHARACTERISTIC"			return TOKEN(TCHARACTERISTIC);

"COMPU_METHOD"				return TOKEN(TCOMPU_METHOD);
"COMPU_TAB"				return TOKEN(TCOMPU_TAB);
"TAB_INTP"				return TOKEN(TTAB_INTP);

"COM_AXIS"				return TOKEN(TCOM_AXIS);
"CURVE"					return TOKEN(TCURVE);
"DEF_CHARACTERISTIC"			return TOKEN(TDEF_CHARACTERISTIC);
"DEPOSIT"				return TOKEN(TDEPOSIT);
"DIRECT"				return TOKEN(TDIRECT);

"FORMAT"				return TOKEN(TFORMAT);
"FUNCTION"				return TOKEN(TFUNCTION);

"STD_AXIS"				return TOKEN(TSTD_AXIS);
"MAP"					return TOKEN(TMAP);
"RECORD_LAYOUT"				return TOKEN(TRECORD_LAYOUT);
"MODULE"				return TOKEN(TMODULE);
"PROJECT"				return TOKEN(TPROJECT);
"HEADER"				return TOKEN(THEADER);
"VERSION"				return TOKEN(TVERSION);
"PROJECT_NO"				return TOKEN(TPROJECT_NO);

"VALUE"					return TOKEN(TVALUE);
"VAL_BLK"				return TOKEN(TVAL_BLK);
"MEASUREMENT"				return TOKEN(TMEASUREMENT);
"REF_CHARACTERISTIC"			return TOKEN(TREF_CHARACTERISTIC);
"IN_MEASUREMENT"			return TOKEN(TIN_MEASUREMENT);
"OUT_MEASUREMENT"			return TOKEN(TOUT_MEASUREMENT);
"LOC_MEASUREMENT"			return TOKEN(TLOC_MEASUREMENT);
"SUB_FUNCTION"				return TOKEN(TSUB_FUNCTION);

"MOD_COMMON"				return TOKEN(TMOD_COMMON);
"MOD_PAR"				return TOKEN(TMOD_PAR);
"BYTE_ORDER"				return TOKEN(TBYTE_ORDER);
"MSB_LAST"				return TOKEN(TMSB_LAST);
"ALIGNMENT_BYTE"			return TOKEN(TALIGNMENT_BYTE);
"ALIGNMENT_WORD"			return TOKEN(TALIGNMENT_WORD);
"ALIGNMENT_LONG"			return TOKEN(TALIGNMENT_LONG);
"MEMORY_SEGMENT"			return TOKEN(TMEMORY_SEGMENT);

"SYSTEM_CONSTANT"			return TOKEN(TSYSTEM_CONSTANT);
"ECU_ADDRESS"				return TOKEN(TECU_ADDRESS);
"BIT_MASK"				return TOKEN(TBIT_MASK);
"NO_AXIS_PTS_X"				return TOKEN(TNO_AXIS_PTS_X);
"NO_AXIS_PTS_Y"				return TOKEN(TNO_AXIS_PTS_Y);
"AXIS_PTS_X"				return TOKEN(TAXIS_PTS_X);
"AXIS_PTS_Y"				return TOKEN(TAXIS_PTS_Y);
"INDEX_INCR"				return TOKEN(TINDEX_INCR);
"FNC_VALUES"				return TOKEN(TFNC_VALUES);
"COLUMN_DIR"				return TOKEN(TCOLUMN_DIR);
"AXIS_PTS_REF"				return TOKEN(TAXIS_PTS_REF);
"FIX_AXIS"				return TOKEN(TFIX_AXIS);
"FIX_AXIS_PAR"				return TOKEN(TFIX_AXIS_PAR);
"B_TRUE"				return TOKEN(TB_TRUE);
"ARRAY_SIZE"				return TOKEN(TARRAY_SIZE);
"READ_ONLY"				return TOKEN(TREAD_ONLY);
"NUMBER"				return TOKEN(TNUMBER);
"RAT_FUNC"				return TOKEN(TRAT_FUNC);
"COEFFS"				return TOKEN(TCOEFFS);
"ASCII"					return TOKEN(TASCII);
"TAB_VERB"				return TOKEN(TTAB_VERB);
"COMPU_TAB_REF"				return TOKEN(TCOMPU_TAB_REF);
"COMPU_VTAB"				return TOKEN(TCOMPU_VTAB);

<INITIAL>{
"/*"					BEGIN(IN_COMMENT);
}

<IN_COMMENT>{
"*/"					BEGIN(INITIAL);
[^*\n]+					// eat comment in chunks
"*"					// eat the lone star
\n					yylineno++;
}


<INITIAL>{
"/begin A2ML"				BEGIN(IN_A2ML); printf("ignoring A2ML\n");
}

<IN_A2ML>{
"/end A2ML"				BEGIN(INITIAL); printf("end A2ML\n");
\n					yylineno++;
.
}


<INITIAL>{
"/begin IF_DATA"			BEGIN(IN_IF_DATA); printf("ignoring IF_DATA\n");
}

<IN_IF_DATA>{
"/end IF_DATA"				BEGIN(INITIAL); printf("end IF_DATA\n");
\n					yylineno++;
.
}


"/begin"				return TOKEN(TLBRACE);
"/end"					return TOKEN(TRBRACE);

"0x"[a-fA-F0-9_]*			SAVE_TOKEN; return TADDRESS;
\"([^\"]|\"\")*\"			SAVE_STRING; return TSTRING;
[a-zA-Z_][a-zA-Z0-9_]* 			SAVE_TOKEN; return TIDENTIFIER;
(-)?[0-9]+\.[0-9]*(e[-+][0-9]*)? 	SAVE_TOKEN; return TDOUBLE;
(-)?[0-9]+				SAVE_TOKEN; return TINTEGER;

.					printf("Unknown token at line %d!\n", yylineno); yyterminate();

%%

/*
"CODE"					return TOKEN(TCODE);
"EPROM"					return TOKEN(TEPROM);
"EXTERN"				return TOKEN(TEXTERN);
"INTERN"				return TOKEN(TINTERN);

"IF_DATA"				return TOKEN(TIF_DATA);
"="					return TOKEN(TEQUAL);
*/
