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

#pragma once

#include <iostream>
#include <vector>
#include <utility>
#include <cstdio>
#include <boost/checked_delete.hpp>
#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/static_assert.hpp>

#include "util.h"
#include "owner_ptr.hpp"
#include <map>

enum AxisStyle { Extern, Intern, Fixed };

class NStatement;
class NExpression;

//class NVariableDeclaration;

class NCharacteristic;

class NBaseMap;

template<class T>
class NMap;

class NCurve;
class NValue;
class NValBlk;
class NCharacteristicText;

class NAxisPts;
class NMeasurement;
class NFunction;
class NCompuMethod;
class NRecordLayout;

class NConstant;
class NVariable;

typedef std::vector<NStatement*> StatementList;
typedef std::vector<NExpression*> ExpressionList;
//typedef std::vector<NVariableDeclaration*> VariableList;

//typedef hash_map<std::string, NStatement*, hash<std::string>, eqstring> StatementHashMap;
typedef boost::unordered_map<std::string, NCharacteristic*> CharacteristicHashMap;
typedef boost::unordered_map<std::string, NAxisPts*> AxisPtsHashMap;
typedef boost::unordered_map<std::string, NMeasurement*> MeasurementHashMap;
typedef boost::unordered_map<std::string, NFunction*> FunctionHashMap;
typedef boost::unordered_map<std::string, NCompuMethod*> CompuMethodHashMap;
typedef boost::unordered_map<std::string, NRecordLayout*> RecordLayoutHashMap;
//typedef boost::unordered_map<std::string, NStatement*> StatementHashMap;

class Visitor
{
public:
    virtual ~Visitor() { }

    // all top-level statements
    //	virtual void visit(NCharacteristic* elem) = 0;
    virtual void visit(/*NMap*/ NBaseMap* elem) = 0;
    virtual void visit(NCurve* elem) = 0;
    virtual void visit(NValue* elem) = 0;
    virtual void visit(NValBlk* elem) = 0;
    virtual void visit(NCharacteristicText* elem) = 0;

    virtual void visit(NAxisPts* elem) = 0;
    virtual void visit(NMeasurement* elem) = 0;
    virtual void visit(NFunction* elem) = 0;
    virtual void visit(NCompuMethod* elem) = 0;
    virtual void visit(NRecordLayout* elem) = 0;

    // inner statements
    virtual void visit(NConstant* elem) = 0;
    virtual void visit(NVariable* elem) = 0;
};

class Node {
public:
    typedef Node my_type;

    Node() : m_parent(0) { }
    virtual ~Node() { }

    const Node* getParent() const
    {
        return m_parent;
    }

    void setParent(const Node* parent)
    {
        m_parent = parent;
    }

    bool hasParent() const
    {
        return (m_parent != 0);
    }

private:
    const Node* m_parent;
};

class NExpression : public Node {
};

class NNumeric : public NExpression {
};

class NInteger : public NNumeric {
public:
    long long value;
    NInteger(long long value) : value(value) { }
};

class NDouble : public NNumeric {
public:
    double value;
    NDouble(double value) : value(value) { }
};

class NAddress : public NNumeric {
public:
    unsigned long value;
    explicit NAddress(unsigned long value) : value(value) { }
    explicit NAddress(const std::string& str)
    {
        std::string addr = getAddressSubstr(str); // remove leading '"0x' and rear '"' characters

        char* p;
        value = strtoul(addr.c_str(), &p, 16); // addresses are hexadecimal
        if (*p != 0) {
            std::cerr << "Invalid address: " << str << std::endl;
        }
    }
};

class NStringLiteral : public NExpression {
public:
    std::string string;
    explicit NStringLiteral(const std::string& string) : string(string) { }
};

class NIdentifier : public NExpression {
public:
    const std::string name;
    NIdentifier(const std::string& name) : name(name) { }
};

class NStatement : public Node {
public:
    owner_ptr<NIdentifier, Node> id;
    NStatement(NIdentifier* id) : id(id, this) { }

    virtual void accept(Visitor& v) = 0;
};

class NBlock : public NExpression {
public:
    StatementList statements;
    //	StatementHashMap statements;

    NBlock() { }

    ~NBlock()
    {
        BOOST_FOREACH (StatementList::value_type i, statements) {
            boost::checked_delete(i);
        }
    }
};



class NVariable : public NStatement {
public:
    NExpression* assignmentExpr; // consider this as weak ref

    NVariable(NIdentifier* id) : NStatement(id) { }

    void accept(Visitor& v) { v.visit(this); }
};

class NConstant : public NStatement {
public:
    const NExpression& assignmentExpr;

    NConstant(NIdentifier* id, const NExpression& assignmentExpr) :
        NStatement(id), assignmentExpr(assignmentExpr) { }

    void accept(Visitor& v) { v.visit(this); }
};

////////////
class NFormat : public NExpression {
public:
    std::string format;
    NFormat(const std::string& format) : format(format) { }

    int getDecimalPl() const;
};

class NAxis : public NExpression { // declaration
public:
    owner_ptr<NIdentifier, Node> m_dataType;
    owner_ptr<NIdentifier, Node> m_compuMethod;
    int length;
    double min;
    double max;

    NAxis(
        NIdentifier* ident,
        NIdentifier* compuMethod,
        int length,
        double min,
        double max) :
        m_dataType(ident, this),
        m_compuMethod(compuMethod, this),
        length(length),
        min(min),
        max(max) { }

    AxisStyle getAxisStyle() const { return m_axisStyle; }

protected:
    AxisStyle m_axisStyle;
};

class NComAxis : public NAxis { // declaration
public:
    owner_ptr<NIdentifier, Node> m_axis_pts;

    NComAxis(
        NIdentifier* dataType,
        NIdentifier* compuMethod,
        int length,
        double min,
        double max,
        NIdentifier* axis_pts) :
        NAxis(dataType, compuMethod, length, min, max),
        m_axis_pts(axis_pts, this)
    { m_axisStyle = Extern; }
};

class NStdAxis : public NAxis { // declaration
public:
    owner_ptr<NFormat, Node> m_format;
    /* NDeposite */

    NStdAxis(
        NIdentifier* dataType,
        NIdentifier* compuMethod,
        int length,
        double min,
        double max,
        NFormat* format) :
        NAxis(dataType, compuMethod, length, min, max),
        m_format(format, this)
    { m_axisStyle = Intern; }
};

class NFixAxis : public NAxis { // declaration
public:
    owner_ptr<NFormat, Node> m_format;
    /* FIX_AXIS_PAR */

    NFixAxis(
        NIdentifier* dataType,
        NIdentifier* compuMethod,
        int length,
        double min,
        double max,
        NFormat* format) :
        NAxis(dataType, compuMethod, length, min, max),
        m_format(format, this)
    { m_axisStyle = Fixed; }
};
//////////////////

class NCharacteristic : public NStatement { // declaration
public:
    std::string description;
    owner_ptr<NAddress, Node> m_address;
    owner_ptr<NIdentifier, Node> m_recordLayout;
    double scale;
    owner_ptr<NIdentifier, Node> m_type;
    double min;
    double max;
    owner_ptr<NFormat, Node, /* optional */ true> m_format;

    NCharacteristic(
        NIdentifier* id,
        const std::string& description,
        NAddress* address,
        NIdentifier* recordLayout,
        double scale,
        NIdentifier* type,
        double min,
        double max,
        NFormat* format) :
        NStatement(id), description(description), m_address(address, this),
        m_recordLayout(recordLayout, this), scale(scale), m_type(type, this),
        min(min), max(max), m_format(format, this)
    { }
};

class NBaseMap : public NCharacteristic {
public:
    NBaseMap(
        NIdentifier* id,
        const std::string& description,
        NAddress* address,
        NIdentifier* recordLayout,
        double scale,
        NIdentifier* type,
        double min,
        double max,
        NFormat* format) :
        NCharacteristic(id, description, address, recordLayout, scale, type, min, max, format)
    { }

    void accept(Visitor& v) { v.visit(this); }

public:
    virtual AxisStyle axisStyle() = 0;
};

NBaseMap* createMap(
    NIdentifier* id,
    const std::string& description,
    NAddress* address,
    NIdentifier* recordLayout,
    double scale,
    NIdentifier* type,
    double min,
    double max,
    NFormat* format,
    NAxis* axis_1,
    NAxis* axis_2);

template<class T>
class NMap : public NBaseMap { // declaration
public:
    owner_ptr<T, Node> m_axis_1;
    owner_ptr<T, Node> m_axis_2;
    typedef T axis_type;

    // achsen müssen den gleichen typen besitzen!
    NMap(
        NIdentifier* id,
        const std::string& description,
        NAddress* address,
        NIdentifier* recordLayout,
        double scale,
        NIdentifier* type,
        double min,
        double max,
        NFormat* format,
        T * axis_1,
        T * axis_2) :
        NBaseMap(id, description, address, recordLayout, scale, type, min, max, format),
        m_axis_1(axis_1, this), m_axis_2(axis_2, this)
    { }

    virtual AxisStyle axisStyle();
};

class NCurve : public NCharacteristic { // declaration
public:
    owner_ptr<NAxis, Node> m_axis_1;

    NCurve(
        NIdentifier* id,
        const std::string& description,
        NAddress* address,
        NIdentifier* recordLayout,
        double scale,
        NIdentifier* type,
        double min,
        double max,
        NFormat* format,
        NAxis* axis_1) :
        NCharacteristic(id, description, address, recordLayout, scale, type, min, max, format),
        m_axis_1(axis_1, this)
    { }

    void accept(Visitor& v) { v.visit(this); }
};

class NValue : public NCharacteristic { // declaration
public:
    NValue(
        NIdentifier* id,
        const std::string& description,
        NAddress* address,
        NIdentifier* recordLayout,
        double scale,
        NIdentifier* type,
        double min,
        double max,
        NFormat* format) :
        NCharacteristic(id, description, address, recordLayout, scale, type, min, max, format)
    { }

    void accept(Visitor& v) { v.visit(this); }
};

class NValBlk : public NCharacteristic { // declaration
public:
    int m_number;

    NValBlk(
        NIdentifier* id,
        const std::string& description,
        NAddress* address,
        NIdentifier* recordLayout,
        double scale,
        NIdentifier* type,
        double min,
        double max,
        NFormat* format,
        int number) :
        NCharacteristic(id, description, address, recordLayout, scale, type, min, max, format),
        m_number(number)
    { }

    void accept(Visitor& v) { v.visit(this); }
};

class NCharacteristicText : public NCharacteristic { // declaration
public:
    int m_size;

    NCharacteristicText(
        NIdentifier* id,
        const std::string& description,
        NAddress* address,
        NIdentifier* recordLayout,
        double scale,
        NIdentifier* type,
        double min,
        double max,
        NFormat* format,
        int size) :
        NCharacteristic(id, description, address, recordLayout, scale, type, min, max, format),
        m_size(size)
    { }

    void accept(Visitor& v) { v.visit(this); }
};
////////

class NMeasurement : public NStatement { // declaration
public:
    std::string description;
    int dataType;
    int int1, int2; // these are always 0 and 100; i don't know for what they are
    owner_ptr<NNumeric, Node> m_min;
    owner_ptr<NNumeric, Node> m_max;
    owner_ptr<NFormat, Node> m_format;
    owner_ptr<NAddress, Node> m_address;

    NMeasurement(
        NIdentifier* id,
        const std::string& description,
        int dataType,
        int int1, int int2,
        NNumeric* min,
        NNumeric* max,
        NFormat* format,
        NAddress* address) :
        NStatement(id), description(description), dataType(dataType),
        int1(int1), int2(int2), m_min(min, this), m_max(max, this),
        m_format(format, this), m_address(address, this)
    { }

    void accept(Visitor& v) { v.visit(this); }
};

class NMeasurementBit : public NMeasurement { // declaration
public:
    std::string bitMask; // TODO: change type

    NMeasurementBit(
        NIdentifier* id,
        const std::string& description,
        int dataType,
        int int1, int int2,
        NNumeric* min,
        NNumeric* max,
        NFormat* format,
        NAddress* address,
        const std::string& bitMask) :
        NMeasurement(id, description, dataType, int1, int2, min, max,format, address),
        bitMask(bitMask)
    { }
};

class NMeasurementValue : public NMeasurement { // declaration
public:
    owner_ptr<NIdentifier, Node> m_type; // samples: dez, t10msxs_ub_b2p55
    // TODO: bitmask
    NMeasurementValue(
        NIdentifier* id,
        const std::string& description,
        int dataType,
        int int1, int int2,
        NNumeric* min,
        NNumeric* max,
        NFormat* format,
        NAddress* address,
        NIdentifier* type) :
        NMeasurement(id, description, dataType, int1, int2, min, max,format, address),
        m_type(type, this)
    { }
};

class NMeasurementArray : public NMeasurementValue { // declaration
public:
    int arraySize;

    NMeasurementArray(
        NIdentifier* id,
        const std::string& description,
        int dataType,
        int int1, int int2,
        NNumeric* min,
        NNumeric* max,
        NFormat* format,
        NAddress* address,
        NIdentifier* type,
        int arraySize) :
        NMeasurementValue(id, description, dataType, int1, int2, min, max, format, address, type),
        arraySize(arraySize)
    { }
};
////////////////

class NAxisPts : public NStatement {
public:
    std::string description;
    const NAddress& address;
    const NIdentifier& unit;
    const NIdentifier& ident;
    double scale;
    const NIdentifier& type;
    int size;
    double min;
    double max;
    const NFormat& format;

    NAxisPts(NIdentifier* id,
             const std::string& description,
             /*	const std::string&*/ const NAddress& address,
             const NIdentifier& units,
             const NIdentifier& ident,
             double scale,
             const NIdentifier& type,
             int size,
             double min,
             double max,
             const NFormat& format) :
        NStatement(id), description(description), address(address),
        unit(unit), ident(ident), scale(scale), type(type), size(size),
        min(min), max(max), format(format) { }

    void accept(Visitor& v) { v.visit(this); }
};
//////////////////


class NCompuMethod : public NStatement {
public:
    std::string description;
    const NFormat& format;
    std::string unit;
    const NNumeric& number1;
    const NNumeric& number2;
    const NNumeric& number3;
    const NNumeric& number4;
    const NNumeric& number5;
    const NNumeric& number6;

    NCompuMethod(NIdentifier* id,
                 const std::string& description,
                 const NFormat& format,
                 const std::string& unit,
                 const NNumeric& number1,
                 const NNumeric& number2,
                 const NNumeric& number3,
                 const NNumeric& number4,
                 const NNumeric& number5,
                 const NNumeric& number6) :
        NStatement(id), description(description), format(format),
        unit(unit), number1(number1), number2(number2),
        number3(number3), number4(number4), number5(number5),
        number6(number6) { }

    void accept(Visitor& v) { v.visit(this); }
};

///////////////////

class NFunction : public NStatement {
public:
    std::string description;

    const ExpressionList& def_characteristic;
    const ExpressionList& ref_characteristic;
    const ExpressionList& in_measurement;
    const ExpressionList& out_measurement;
    const ExpressionList& loc_measurement;
    const ExpressionList& sub_function;

    NFunction(NIdentifier* id,
              const std::string& description,
              const ExpressionList& def_characteristic,
              const ExpressionList& ref_characteristic,
              const ExpressionList& in_measurement,
              const ExpressionList& out_measurement,
              const ExpressionList& loc_measurement,
              const ExpressionList& sub_function) :
        NStatement(id), description(description), def_characteristic(def_characteristic),
        ref_characteristic(ref_characteristic), in_measurement(in_measurement),
        out_measurement(out_measurement), loc_measurement(loc_measurement),
        sub_function(sub_function) { }

    void accept(Visitor& v) { v.visit(this); }
};

///////////////

class NRecordLayout : public NStatement {
private:
    class RecordMember : public Node { };
    enum MemberType { XAxis, YAxis, Fnc };
    typedef boost::shared_ptr<RecordMember> RecordPtr;
    std::map<MemberType, RecordPtr> m_members; // RECORD_LAYOUT members vary in all posible ways

public:
    NRecordLayout(NIdentifier* id) :
        NStatement(id) { }

    class AxisLayout : public RecordMember
    {
    public:
        AxisLayout(int NoAxisType, int ValAxisType, int flags) :
            NoAxisType(NoAxisType),
            ValAxisType(ValAxisType),
            flags(flags) { }
        //        int token;
        int NoAxisType;
        int ValAxisType;
        int flags; // z.b. INDEX_INCR DIRECT
    };

    class FncValues : public RecordMember
    {
    public:
        FncValues(int type, int flags) :
            type(type),
            flags(flags) { }
        int type;
        int flags; // z.b. COLUMN_DIR DIRECT
    };

    void accept(Visitor& v) { v.visit(this); }

    bool hasFncValues() const { return true; } // FIXME
    bool hasXAxis() const;
    bool hasYAxis() const;

    const AxisLayout& getXAxis() const;
    const AxisLayout& getYAxis() const;
    const FncValues& getFncValues() const;

    // static members
    static NRecordLayout* createRecordLayout(
        NIdentifier* id,
        NRecordLayout::FncValues* fncValues);

    static NRecordLayout* createRecordLayout(
        NIdentifier* id,
        int NoAxisTypeX,
        int ValAxisTypeX,
        int AxisFlagsX,
        NRecordLayout::FncValues* fncValues);

    static NRecordLayout* createRecordLayout(
        NIdentifier* id,
        int NoAxisTypeX,
        int ValAxisTypeX,
        int AxisFlagsX,
        int NoAxisTypeY,
        int ValAxisTypeY,
        int AxisFlagsY,
        NRecordLayout::FncValues* fncValues);
};
///////////////

class NHeader : public NExpression {
public:
    std::string name;
    std::string model;
    const NIdentifier& project; // std::auto_ptr für alle referenzen?

    NHeader(const std::string& name, const std::string& model, const NIdentifier& project) :
        name(name), model(model), project(project) { }
};

class NModule : public Node, public Visitor {
public:
    const NBlock& innerBlock;

    CharacteristicHashMap characteristics;
    AxisPtsHashMap axisPts;
    MeasurementHashMap measurements;
    FunctionHashMap functions;
    CompuMethodHashMap compuMethods;
    RecordLayoutHashMap recordLayouts;
    //	| compu_tab
    //	| compu_vtab

    NModule(const NBlock& innerBlock) : innerBlock(innerBlock) { buildMaps(); }

    //	void visit(NCharacteristic* elem)    { characteristics[elem->id.name] = elem; }
    void visit(/*NMap*/ NBaseMap* elem)      { characteristics[elem->id->name] = elem; }
    void visit(NCurve* elem)                 { characteristics[elem->id->name] = elem; }
    void visit(NValue* elem)                 { characteristics[elem->id->name] = elem; }
    void visit(NValBlk* elem)                { characteristics[elem->id->name] = elem; }
    void visit(NCharacteristicText* elem)    { characteristics[elem->id->name] = elem; }

    void visit(NAxisPts* elem)               { axisPts[elem->id->name] = elem; }
    void visit(NMeasurement* elem)           { measurements[elem->id->name] = elem; }
    void visit(NFunction* elem)              { functions[elem->id->name] = elem; }
    void visit(NCompuMethod* elem)           { compuMethods[elem->id->name] = elem; }
    void visit(NRecordLayout* elem)          { recordLayouts[elem->id->name] = elem; }

    // inner statements
    void visit(NConstant* elem) { std::cerr << "NConstant is invalid in this context!\n" << std::endl; }
    void visit(NVariable* elem) { std::cerr << "NVariable is invalid in this context!\n" << std::endl; }

private:
    void buildMaps()
    {
        BOOST_FOREACH(StatementList::value_type i, innerBlock.statements) {
            if (i != NULL) i->accept(*this);
        }
    }
};

class NProject : public Node {
public:
    const NHeader& header;
    const NModule& module;
    NProject(const NHeader& header, const NModule& module) : header(header), module(module) { }
};
