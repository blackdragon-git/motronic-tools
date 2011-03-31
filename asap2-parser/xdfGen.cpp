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
/*
void Node::xdfGen() const
{
//	cout << "Node" << endl;
}

void NMap::xdfGen() const
{
	std::cout << "Generate xdf for " << id.name << std::endl;
}
*/

XdfGen::XdfGen(const NModule& module) :
    m_done(false), m_module(module), m_xdf(std::stringstream::out)
{
    createHeader();
}

void XdfGen::createCategorys()
{
    int n = 0;
    m_xdf << std::hex; //.setf(std::ios::hex);

    const FunctionHashMap& functions = m_module.functions;
    BOOST_FOREACH (FunctionHashMap::value_type i, functions) {
        const std::string& name = i.second->id->name;

        m_categorys[name] = n; // save our xdf-id
        m_xdf << "<CATEGORY index=\"0x" << n << "\" "
              << "name=\"" << name << ": "
              << i.second->description << "\" />\n";
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
            m_xdf << "<CATEGORYMEM index=\"0\" category=\"" // TODO index
                  << m_categorys[func_id.name] + 1 // the reference is the index + 1 in decimal
                  << "\" />\n";
        }
    }
}

void XdfGen::createCatRefsForMap(const NIdentifier& id)
{
    const FunctionHashMap& functions = m_module.functions;
    BOOST_FOREACH (FunctionHashMap::value_type i, functions) {

        createCategoryReferences(id, *i.second->id, i.second->def_characteristic);
        createCategoryReferences(id, *i.second->id, i.second->ref_characteristic);
    }
}

void XdfGen::createHeader()
{
    m_xdf << "<XDFFORMAT version=\"1.50\">" << "\n"
          << "<XDFHEADER>" << "\n"
          << "<flags>0x1</flags>" << "\n"
          << "<fileversion>Version 1</fileversion>" << "\n"
          << "<deftitle>data/06A906032BJ-0261206892</deftitle>" << "\n"
          << "<description>Original</description>" << "\n"
          << "<author>generated</author>" << "\n"
          << "<baseoffset>0</baseoffset>" << "\n"
          << "<DEFAULTS datasizeinbits=\"8\" sigdigits=\"2\" outputtype=\"1\" signed=\"0\" lsbfirst=\"0\" float=\"0\" />" << "\n"
          << "<REGION type=\"0xFFFFFFFF\" startaddress=\"0x0\" size=\"0x100000\" regionflags=\"0x0\" name=\"Binary File\" desc=\"This region describes the bin file edited by this XDF\" />" << "\n";

    createCategorys();

    m_xdf << "</XDFHEADER>" << "\n";
}

void XdfGen::createFinalAxis(const char* name)
{
    m_xdf << "<XDFAXIS id=\"" << name << "\">" << "\n"
          << "</XDFAXIS>" << "\n";
}
/*
<MATH equation="0.750000 * X+ -48.000000">
  <VAR id="X" />
</MATH>*/
// TODO inline
void createMathEquation(//const NAxis& axis,
                                short typeSize,
                                bool typeSign,
                 double max, double min,
                                double& factor, /* out */
                                double& offset /* out */)
{
//    short typeSize;
//    bool typeSign;
    int typeMax;
    if (typeSign) {
        typeMax = (1 << (typeSize - 1)) - 1;
    }
    else {
        typeMax = (1 << typeSize) - 1;
    }
    double district = std::abs(max - min);
    factor = district / typeMax;
    offset = min;
    //     std::stringstream
}

void XdfGen::epilogue()
{
    if (m_done) return;

    m_xdf << "</XDFFORMAT>" << std::endl;
    m_done = true;
    // TESTING
    std::cout << m_xdf.str() << std::endl;
}

unsigned int XdfGen::handleAxis(const NAxis& axis, // TODO add col or row
                                unsigned int baseAddr, //const NAddress& baseAddr,
                                const char* name)
{
    const NMeasurement* measurement = m_module.measurements.at(axis.m_dataType->name);
    assert(measurement != NULL);

    short typeSize;
    bool typeSign;
    getDataTypeInfo(measurement->dataType, &typeSize, &typeSign);

    unsigned int startAddr, offset;

    AxisStyle axisStyle = axis.getAxisStyle();
    if (axisStyle == Extern) {
        const NComAxis* comAxis = dynamic_cast<const NComAxis*>(&axis);
        assert (comAxis != NULL);
        std::cout << "handle com axis" << std::endl;
        offset = 0; // a com-axis does not affect our map address
        const NAxisPts* axisPts = m_module.axisPts.at(comAxis->axis_pts.name);
        startAddr = axisPts->address.value;

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

    //generate:
    m_xdf << "<XDFAXIS id=\"" << name << "\" uniqueid=\"0x0\">" << "\n" // TODO uniqueid
          << "<EMBEDDEDDATA mmedtypeflags=\"0x02\" "
          << "mmedaddress=\"0x" << std::hex << startAddr << std::dec << "\" "
          << "mmedelementsizebits=\"" << typeSize << "\" "
          << "mmedcolcount=\"" << axis.length << "\" "
          << "mmedmajorstridebits=\"" << typeSize << "\" " // should be the same as mmedelementsizebits
          << "/>\n"
          << "<units>" << compuMethod->unit << "</units>\n"
          << "<indexcount>" << axis.length << "</indexcount>\n"
          << "<decimalpl>" << compuMethod->format.getDecimalPl() << "</decimalpl>\n"
          << "<embedinfo type=\"1\" />\n"
          << "<datatype>0</datatype>\n"
          << "<unittype>0</unittype>\n"
          << "<DALINK index=\"0\" />\n";

    double factor, d_offset;
    createMathEquation(typeSize, typeSign, axis.max, axis.min, factor, d_offset);

    m_xdf << "<MATH equation=\"" << factor << " * X";

    if (d_offset != 0) m_xdf << "+ " << d_offset;

    m_xdf << "\">\n"
          << "<VAR id=\"X\" />\n"
          << "</MATH>\n"
          << "</XDFAXIS>" << "\n";

    return offset;
}

// all top-level statements
void XdfGen::visit(/*NMap*/NBaseMap* elem)
{
    std::cout << "visiting NMap " << elem->id->name << std::endl;

    m_xdf << "<XDFTABLE uniqueid=\"0x0\" flags=\"0x0\">" << "\n"
          << "<title>" << elem->id->name << "</title>" << "\n"
          << "<description>" << elem->description << "</description>" << "\n"; // TODO: umlaute!

    unsigned int offset = 0;
    if (elem->axisStyle() == Intern) {
        std::cout << "with std-axis\n";
        const NMap<NStdAxis>* stdMap = dynamic_cast<const NMap<NStdAxis>*>(elem);
        offset += handleStdMap(stdMap);
    }
    else if (elem->axisStyle() == Extern) {
        std::cout << "with com-axis\n";
        const NMap<NComAxis>* comMap = dynamic_cast<const NMap<NComAxis>*>(elem);
        handleComMap(comMap);
    }
    else if (elem->axisStyle() == Fixed) {
        std::cout << "with fix-axis\n";
    }
// RECORD_LAYOUT ?
    short typeSize;
    bool typeSign;
//    std::cout << "RECORD " << elem->m_recordLayout.name << std::endl;
    const NRecordLayout* recordLayout = m_module.recordLayouts.at(elem->m_recordLayout->name);

    if (!recordLayout->hasFncValues()) {
        std::cerr << "NRecordLayout for map: " << elem->id->name
                  << " should have an FNC_VALUES entry!" << std::endl;
        throw std::exception();
    }
    getDataTypeInfo(recordLayout->getFncValues().type, &typeSize, &typeSign);

    createFinalAxis("z");
    m_xdf << "</XDFTABLE>" << "\n";
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

unsigned int XdfGen::handleStdMap(const NMap<NStdAxis>* stdMap)
{
    assert(stdMap != NULL);
    unsigned int offset = 2, axisAddr; // the first two bytes to describe the axis length

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
