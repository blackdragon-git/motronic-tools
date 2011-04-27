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

#include <cassert>
#include <cmath>

#include <iostream>

#include "xdfGen.h"
#include "util.h"

XdfGen::XdfGen(
    const NModule& module,
    int offset) :
    m_done(false),
    m_module(module),
    m_offset(offset),
    m_baseStream(std::stringstream::out),
    m_xdf(m_baseStream)
{
    createHeader();
}

void XdfGen::createCategorys()
{
    int n = 0;
    m_xdf << std::hex;

    const FunctionHashMap& functions = m_module.functions;
    BOOST_FOREACH (FunctionHashMap::value_type i, functions) {
        const std::string& name = i.second->id->name;

        m_categorys[name] = n; // save our xdf-id

        m_xdf << xml::startTag("CATEGORY") << xml::attribute("index") << "0x" << n
              << xml::attribute("name") << name << ": "
              << i.second->description << xml::endTag;
        ++n;
    }

    m_xdf << std::dec;
}

void XdfGen::createCategoryReferences(const NIdentifier& id,
                                      const NIdentifier& func_id,
                                      const ExpressionList& refs)
{
    m_xdf << std::dec;

    // iterate throug our identifiers in refs
    BOOST_FOREACH (ExpressionList::value_type i, refs) {

        // its not guaranteed to be an NIdentifier
        const NIdentifier* d_id = dynamic_cast<const NIdentifier*>(i);
        if (d_id != NULL && id.name == d_id->name) {
            m_xdf << xml::startTag("CATEGORYMEM") << xml::attribute("index") << 0 // TODO index
                  << xml::attribute("category")
                  << m_categorys[func_id.name] + 1 // the reference is the index + 1 in decimal
                  << xml::endTag;
        }
    }
}

void XdfGen::createCatRefsForMap(const NIdentifier& id)
{
    const FunctionHashMap& functions = m_module.functions;
    BOOST_FOREACH (FunctionHashMap::value_type i, functions) {

        createCategoryReferences(id, * i.second->id, * i.second->def_characteristic);
        createCategoryReferences(id, * i.second->id, * i.second->ref_characteristic);
    }
}

void XdfGen::createHeader()
{
    m_xdf << xml::startTag("XDFFORMAT") << xml::attribute("version") << "1.50"
          << xml::startTag("XDFHEADER")
          << xml::startTag("flags") << xml::content << "0x1"<< xml::endTag
          << xml::startTag("fileversion") << xml::content << "Version 1" << xml::endTag
          << xml::startTag("deftitle") << xml::content << "EPK" << xml::endTag // TODO
          << xml::startTag("description") << xml::content << "EPK" << xml::endTag // TODO
          << xml::startTag("author") << xml::content << "generated" << xml::endTag
          << xml::startTag("baseoffset") << xml::content << 0 << xml::endTag
          << xml::startTag("DEFAULTS") << xml::attribute("datasizeinbits") << 8
          << xml::attribute("sigdigits") << 2 << xml::attribute("outputtype") << 1
          << xml::attribute("signed") << 0 << xml::attribute("lsbfirst")
          << xml::attribute("float") << 0 << xml::endTag
          << xml::startTag("REGION") << xml::attribute("type") << "0xFFFFFFFF"
          << xml::attribute("startaddress") << "0x0"
          << xml::attribute("size") << "0x100000" // TODO
          << xml::attribute("regionflags") << "0x0"
          << xml::attribute("name") << "Binary File"
          << xml::attribute("desc") << "This region describes the bin file edited by this XDF"
          << xml::endTag; // close region tag

    createCategorys();

    m_xdf << xml::endTag; // close header tag
}

void XdfGen::createMathEquation(
    short typeSize,
    bool typeSign,
    double max, double min)
{
    float factor, offset, district;
    int typeMax;

    typeMax = (1 << typeSize) - 1;
    district = std::abs(max - min);
    factor = district / typeMax;
    if (!typeSign) offset = min;

    m_xdf << xml::startTag("MATH") << xml::attribute("equation") << factor << " * X";

    if (offset != 0) m_xdf << "+ " << offset;

    m_xdf << xml::startTag("VAR") << xml::attribute("id") << "X" << xml::endTag
          << xml::endTag;
}

void XdfGen::epilogue()
{
    if (m_done) return;

    m_xdf << xml::endTag;
    m_done = true;
    // TESTING
    std::cout << m_xdf.str() << std::endl;
}

static inline int getTypeFlags(bool msbLast, bool typeSign)
{
    int typeFlags = 0;
    if (msbLast) typeFlags |= 0x2; // little endian
    if (typeSign) typeFlags |= 0x1;

    return typeFlags;
}

unsigned int XdfGen::handleAxis(
    const NAxis& axis,
    unsigned int baseAddr,
    const char* name)
{
    const NMeasurement* measurement = m_module.measurements.at(axis.m_dataType->name);
    assert(measurement != NULL);

    short typeSize;
    bool typeSign, msbLast = true; // TODO: endianness
    getDataTypeInfo(measurement->dataType, &typeSize, &typeSign);

    int offset = m_offset;
    unsigned int startAddr;

    AxisStyle axisStyle = axis.getAxisStyle();
    if (axisStyle == Extern) {
        const NComAxis* comAxis = dynamic_cast<const NComAxis*>(&axis);
        assert(comAxis != NULL);
        std::cout << "handle com axis" << std::endl;
        offset = 0; // a com-axis does not affect our map address
        const NAxisPts* axisPts = m_module.axisPts.at(comAxis->m_axis_pts->name);
        startAddr = axisPts->m_address->value;
    }
    else if (axisStyle == Intern) {
        const NStdAxis* stdAxis = dynamic_cast<const NStdAxis*>(&axis);
        assert(stdAxis != NULL);
        std::cout << "handle std axis" << std::endl;
        startAddr = baseAddr + offset;
        offset = (stdAxis->length * typeSize) / 8; // gesamtgröße
    }
    else if (axisStyle == Fixed) {
        const NFixAxis* fixAxis = dynamic_cast<const NFixAxis*>(&axis);
        assert(fixAxis != NULL);
        std::cout << "handle fix axis" << std::endl;
        offset += 0; // a fix-axis does not affect our map address
    }

    const NCompuMethod* compuMethod = m_module.compuMethods.at(axis.m_compuMethod->name);
    assert(compuMethod != NULL);

    std::string units = compuMethod->unit;
    if (units.empty()) units = "-";

    // generate:
    m_xdf << xml::startTag("XDFAXIS") << xml::attribute("id") << name
          << xml::attribute("uniqueid") << "0x0" // TODO uniqueid
          << xml::startTag("EMBEDDEDDATA") << xml::attribute("mmedtypeflags")
          << std::hex << "0x" << getTypeFlags(msbLast, typeSign)
          << xml::attribute("mmedaddress") << "0x" << startAddr << std::dec
          << xml::attribute("mmedelementsizebits") << typeSize
          << xml::attribute("mmedcolcount") << axis.length
          << xml::attribute("mmedmajorstridebits") << typeSize // should be the same as mmedelementsizebits
          << xml::endTag
          << xml::startTag("units") << xml::content << units << xml::endTag
          << xml::startTag("indexcount") << xml::content << axis.length << xml::endTag
          << xml::startTag("decimalpl") << xml::content
          << compuMethod->m_format->getDecimalPl() << xml::endTag
          << xml::startTag("embedinfo") << xml::attribute("type") << 1 << xml::endTag
          << xml::startTag("datatype") << xml::content << 0 << xml::endTag
          << xml::startTag("unittype") << xml::content << 0 << xml::endTag
          << xml::startTag("DALINK") << xml::attribute("index") << 0 << xml::endTag;

//    double factor, d_offset;
    createMathEquation(typeSize, typeSign, axis.max, axis.min);//, factor, d_offset);

    m_xdf << xml::endTag;

    return offset;
}

// all top-level statements
void XdfGen::visit(NBaseMap* elem)
{
    std::cout << "visiting NMap " << elem->id->name << std::endl;

    m_xdf << xml::startTag("XDFTABLE")
          << xml::attribute("uniqueid") << "0x0" // TODO
          << xml::attribute("falgs") << "0x0"
          << xml::startTag("title") << xml::content << elem->id->name << xml::endTag
          << xml::startTag("description") << xml::content << elem->description << xml::endTag; // TODO: umlaute!

    const NRecordLayout* recordLayout = m_module.recordLayouts.at(elem->m_recordLayout->name);
    assert(recordLayout != NULL);

    if (!recordLayout->hasFncValues()) {
        std::cerr << "NRecordLayout for the map: " << elem->id->name
                  << " should have an FNC_VALUES entry!" << std::endl;
        throw std::exception();
    }

    int offset = m_offset;
    unsigned int startAddr;
    if (elem->axisStyle() == Intern) {
        std::cout << "with std-axis\n";
        const NMap<NStdAxis>* stdMap = dynamic_cast<const NMap<NStdAxis>*>(elem);
        offset += handleStdMap(stdMap, *recordLayout);
    }
    else if (elem->axisStyle() == Extern) {
        std::cout << "with com-axis\n";
        const NMap<NComAxis>* comMap = dynamic_cast<const NMap<NComAxis>*>(elem);
        handleComMap(comMap);
    }
    else if (elem->axisStyle() == Fixed) {
        std::cout << "with fix-axis\n";
    }

    // final data address
    startAddr = elem->m_address->value + offset;

    short typeSize;
    bool typeSign, msbLast = true; // TODO: endianness
    getDataTypeInfo(recordLayout->getFncValues().type, &typeSize, &typeSign);

    // CompuMethod data:
    const NCompuMethod* compuMethod = m_module.compuMethods.at(elem->m_compuMethod->name);
    assert(compuMethod != NULL);

    std::string units = compuMethod->unit;
    if (units.empty()) units = "-";

    // create final Axis
    m_xdf << xml::startTag("XDFAXIS") << xml::attribute("id") << "z"
          << xml::startTag("EMBEDDEDDATA") << xml::attribute("mmedtypeflags")
          << std::hex << "0x" << getTypeFlags(msbLast, typeSign)
          << xml::attribute("mmedaddress") << "0x" << startAddr << std::dec
          << xml::attribute("mmedelementsizebits") << typeSize
          << xml::attribute("mmedrowcount") << elem->axisXlength()
          << xml::attribute("mmedcolcount") << elem->axisYlength()
          << xml::endTag
          << xml::startTag("units") << xml::content(units) << xml::endTag
          << xml::startTag("decimalpl") << xml::content << elem->m_format->getDecimalPl() << xml::endTag
          << xml::startTag("min") << xml::content << elem->min << xml::endTag
          << xml::startTag("max") << xml::content << elem->max << xml::endTag
          << xml::startTag("outputtype") << xml::content << 1 << xml::endTag;

    createMathEquation(typeSize, typeSign, elem->max, elem->min);

    m_xdf << xml::endTag(2);
}

void XdfGen::handleComMap(const NMap<NComAxis>* comMap)
{
    assert(comMap != NULL);

    createCatRefsForMap(*comMap->id);
    try {
        handleAxis(*comMap->m_axis_1.get(), comMap->m_address->value, "x");
        handleAxis(*comMap->m_axis_2.get(), comMap->m_address->value, "y");
    }
    catch (std::out_of_range& e) {
//    catch (std::exception& e) {
        std::cerr << "std::out_of_range exception in handleComMap" << std::endl;
    }
}

unsigned int XdfGen::handleStdMap(
    const NMap<NStdAxis>* stdMap,
    const NRecordLayout& recordLayout)
{
    assert(stdMap != NULL);

    if (!recordLayout.hasXAxis() || !recordLayout.hasYAxis()) {
        std::cerr << "NRecordLayout for the map: " << stdMap->id->name
                  << " is missing the axis description!" << std::endl;
        throw std::exception();
    }

    int noTypeX = recordLayout.getXAxis().NoAxisType;
    int noTypeY = recordLayout.getYAxis().NoAxisType;

    short typeSizeX, typeSizeY;
    bool typeSignX, typeSignY;
    getDataTypeInfo(noTypeX, &typeSizeX, &typeSignX);
    getDataTypeInfo(noTypeY, &typeSizeY, &typeSignY);

    // calculate the start address of the axis description
    unsigned int offset = (typeSizeX + typeSignY) / 8;
    unsigned int axisAddr;

    createCatRefsForMap(*stdMap->id);
    try {
        axisAddr = stdMap->m_address->value + offset;
        offset += handleAxis(*stdMap->m_axis_1.get(), axisAddr, "x");
        axisAddr = stdMap->m_address->value + offset; // adjust with new offset
        offset += handleAxis(*stdMap->m_axis_2.get(), axisAddr, "y");
    }
    catch (std::out_of_range& e) {
//    catch (std::exception& e) {
        std::cerr << "std::out_of_range exception in handleStdMap" << std::endl;
    }

    return offset;
}

void XdfGen::handleFixMap(const NMap<NFixAxis>* fixMap)
{
    assert(fixMap != NULL);

    std::cout << "handle fix map" << std::endl;
}

void XdfGen::visit(NCurve* elem)
{
    std::cout << "visiting NCurve " << elem->id->name << std::endl;

}

void XdfGen::visit(NValue* elem)
{

}

void XdfGen::visit(NValBlk* elem)
{

}

void XdfGen::visit(NCharacteristicText* elem)
{

}

void XdfGen::visit(NAxisPts* elem)
{

}

void XdfGen::visit(NMeasurement* elem)
{

}

void XdfGen::visit(NFunction* elem)
{

}

void XdfGen::visit(NCompuMethod* elem)
{

}

void XdfGen::visit(NRecordLayout* elem)
{

}

// inner statements
void XdfGen::visit(NConstant* elem)
{
    printf("NConstant is invalid in this context!\n");
}

void XdfGen::visit(NVariable* elem)
{
    printf("NVariable is invalid in this context!\n");
}
