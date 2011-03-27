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

#include "node.h"

int NFormat::getDecimalPl() const
{
    int pos = format.find('.');
    if (pos == format.npos) throw std::invalid_argument("Format string");

    ++pos;
    if (pos >= format.length()) throw std::out_of_range("Format string");

    int decimalPl = atoi(format.substr(pos).c_str());
    return decimalPl;
}

// specialization for our axis-types
template<>
AxisStyle NMap<NComAxis>::axisStyle()
{
    return Extern;
}

template<>
AxisStyle NMap<NStdAxis>::axisStyle()
{
    return Intern;
}

template<>
AxisStyle NMap<NFixAxis>::axisStyle()
{
    return Fixed;
}

NBaseMap* createMap(const NIdentifier& name,
                    const std::string& description,
                    const NAddress& address,
                    const NIdentifier& recordLayout,
                    double scale,
                    const NIdentifier& type,
                    double min,
                    double max,
                    const NFormat& format,
                    const NAxis& axis_1,
                    const NAxis& axis_2)
{
    AxisStyle style1 = axis_1.getAxisStyle();
    AxisStyle style2 = axis_1.getAxisStyle();

    if (style1 != style2) {
        std::cerr << "createMap got different axes in one map!" << std::endl;
        return NULL;
    }

    NBaseMap* map;
    if (style1 == Extern) {
        map = new NMap<NComAxis>(name,
                                 description,
                                 address,
                                 recordLayout,
                                 scale,
                                 type,
                                 min,
                                 max,
                                 format,
                                 dynamic_cast<const NComAxis&>(axis_1), // this cast is always save
                                 dynamic_cast<const NComAxis&>(axis_2));
    }
    else if (style1 == Intern) {
        map = new NMap<NStdAxis>(name,
                                 description,
                                 address,
                                 recordLayout,
                                 scale,
                                 type,
                                 min,
                                 max,
                                 format,
                                 dynamic_cast<const NStdAxis&>(axis_1), // this cast is always save
                                 dynamic_cast<const NStdAxis&>(axis_2));
    }
    else if (style1 == Fixed) {
        map = new NMap<NFixAxis>(name,
                                 description,
                                 address,
                                 recordLayout,
                                 scale,
                                 type,
                                 min,
                                 max,
                                 format,
                                 dynamic_cast<const NFixAxis&>(axis_1), // this cast is always save
                                 dynamic_cast<const NFixAxis&>(axis_2));
    }
    return map;
}

const NRecordLayout::AxisLayout& NRecordLayout::getXAxis() const
{
    // if this cast fails somthing is terribly wrong
    // so let the caller handle this...
    return dynamic_cast<const AxisLayout&>(*m_members.at(XAxis).get());
}

const NRecordLayout::AxisLayout& NRecordLayout::getYAxis() const
{
    // same as above
    return dynamic_cast<const AxisLayout&>(*m_members.at(YAxis).get());
}

const NRecordLayout::FncValues& NRecordLayout::getFncValues() const
{
    // same as above
    return dynamic_cast<const FncValues&>(*m_members.at(Fnc).get());
}

NRecordLayout* NRecordLayout::createRecordLayout(
    const NIdentifier& name,
    NRecordLayout::FncValues* fncValues)
{
    if (fncValues == NULL) {
        std::cerr << "The given RECORD_LAYOUT is empty!" << std::endl;
        return NULL;
    }

    // the simplest case is only a FNC_VALUES record
    NRecordLayout* recordLayout = new NRecordLayout(name);
    recordLayout->m_members[NRecordLayout::Fnc] = RecordPtr(fncValues);

    return recordLayout;
}
/*
NRecordLayout* NRecordLayout::createRecordLayout(
    const NIdentifier& name,
    int NoAxisTypeX,
    int ValAxisTypeX,
    int AxisFlagsX)
{

}
*/
NRecordLayout* NRecordLayout::createRecordLayout(
    const NIdentifier& name,
    int NoAxisTypeX,
    int ValAxisTypeX,
    int AxisFlagsX,
    NRecordLayout::FncValues* fncValues)
{
    NRecordLayout* recordLayout = new NRecordLayout(name);

    // a fixed curve may be defined without FNC_VALUES
    if (fncValues != NULL) {
        recordLayout->m_members[NRecordLayout::Fnc] = RecordPtr(fncValues);
    }

    AxisLayout* xAxis = new AxisLayout(NoAxisTypeX, ValAxisTypeX, AxisFlagsX);
    recordLayout->m_members[NRecordLayout::XAxis] = RecordPtr(xAxis);

    return recordLayout;
}

NRecordLayout* NRecordLayout::createRecordLayout(
    const NIdentifier& name,
    int NoAxisTypeX,
    int ValAxisTypeX,
    int AxisFlagsX,
    int NoAxisTypeY,
    int ValAxisTypeY,
    int AxisFlagsY,
    NRecordLayout::FncValues* fncValues)
{
    if (fncValues == NULL) {
        std::cerr << "A RECORD_LAYOUT for a map should have FNC_VALUES!" << std::endl;
        return NULL;
    }

    NRecordLayout* recordLayout = new NRecordLayout(name);

    AxisLayout* xAxis = new AxisLayout(NoAxisTypeX, ValAxisTypeX, AxisFlagsX);
    AxisLayout* yAxis = new AxisLayout(NoAxisTypeY, ValAxisTypeY, AxisFlagsY);

    recordLayout->m_members[NRecordLayout::XAxis] = RecordPtr(xAxis);
    recordLayout->m_members[NRecordLayout::YAxis] = RecordPtr(yAxis);
    recordLayout->m_members[NRecordLayout::Fnc] = RecordPtr(fncValues);

    return recordLayout;
}