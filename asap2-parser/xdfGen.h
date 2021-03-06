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

#include <string>
#include <sstream>

#include <boost/foreach.hpp>
#include <boost/unordered_map.hpp>

#include "node.h"
#include "XmlStream.hpp"

using namespace xml;

class XdfGen : public Visitor
{
public:
    XdfGen(
        const NModule& module,
        int offset = 0);

    virtual ~XdfGen() { }

    void epilogue();

    // all top-level statements
    void visit(NBaseMap* elem);
    void visit(NCurve* elem);
    void visit(NValue* elem);
    void visit(NValBlk* elem);
    void visit(NCharacteristicText* elem);

    void visit(NAxisPts* elem);
    void visit(NMeasurement* elem);
    void visit(NFunction* elem);
    void visit(NCompuMethod* elem);
    void visit(NRecordLayout* elem);

    // inner statements
    void visit(NConstant* elem);
    void visit(NVariable* elem);

private:
    void createCategorys();

    void createCategoryReferences(
        const NIdentifier& id,
        const NIdentifier& func_id,
        const ExpressionList& refs);

    void createCatRefsForMap(const NIdentifier& id);

    void createHeader();

    void createMathEquation(
        short typeSize,
        bool typeSign,
        double max, double min);

    unsigned int handleAxis(
        const NAxis& axis,
        unsigned int baseAddr,
        const char* name);

    void handleComMap(const NMap<NComAxis>* comMap);

    unsigned int handleStdMap(
        const NMap<NStdAxis>* stdMap,
        const NRecordLayout& recordLayout);

    void handleFixMap(const NMap<NFixAxis>* fixMap);

    typedef boost::unordered_map<std::string, int> CategorysHashMap;

    // members:
    CategorysHashMap m_categorys;
    bool m_done;
    const NModule& m_module;
    int m_offset;

    std::stringstream m_baseStream;
    XmlStream<char> m_xdf;
};
