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

#include <sstream>
#include <stack>

namespace xml
{
using namespace std;

struct XmlElement
{ };

template<class Ch, class Tr = char_traits<Ch> >
struct XmlAttribute : public XmlElement
{
    const basic_string<Ch, Tr>& m_name;
    const basic_string<Ch, Tr>& m_content;

    XmlAttribute(
        const basic_string<Ch, Tr>& name,
        const basic_string<Ch, Tr>& content) :
        m_name(name),
        m_content(content)
    { }

    // copy constructor
    XmlAttribute(const XmlAttribute& attribute) :
        m_name(attribute.m_name),
        m_content(attribute.m_content)
    { }
};

template<class Ch, class Tr = char_traits<Ch> >
struct XmlContent : public XmlElement
{
    const basic_string<Ch, Tr>& m_content;

    XmlContent(const basic_string<Ch, Tr>& content) :
        m_content(content)
    { }

    // copy constructor
    XmlContent(const XmlContent& content) :
        m_content(content.m_content)
    { }
};

template<class Ch, class Tr = char_traits<Ch> >
struct XmlStartTag : public XmlElement
{
    const basic_string<Ch, Tr>& m_name;

    XmlStartTag(const basic_string<Ch, Tr> name) :
        m_name(name)
    { }

    // copy constructor
    XmlStartTag(const XmlStartTag& startTag) :
        m_name(startTag.m_name)
    { }
};

struct XmlEndTag : public XmlElement
{ };

template<class Ch, class Tr = char_traits<Ch> >
class XmlStream //: public basic_stringstream<Ch, Tr>
{
public:
    typedef XmlStream my_type;
    typedef basic_stringstream<Ch, Tr> stream_type;

    enum State { InContent, InAttributeBlock, None };

    XmlStream(stream_type& stream) :
        m_stream(stream),
        m_state(None),
        m_tags()
    { }

    my_type& operator <<(const XmlAttribute<Ch, Tr>& attribute)
    {
        if (m_state != InAttributeBlock) {
            throw ios_base::failure("not in attribute block");
        }

        m_stream << ' ' << attribute.m_name << "=\""
                 << attribute.m_content << '"';
    }

    my_type& operator <<(const XmlContent<Ch, Tr>& content)
    {
        if (m_state == InAttributeBlock) {
            m_stream << '>';
        }

        m_stream << content.m_content;
        m_state = InContent;
    }

    my_type& operator <<(const XmlStartTag<Ch, Tr>& startTag)
    {
        int i = m_tags.size();
        for (; i != 0; --i) m_stream << "  ";

        m_stream << '<' << startTag.m_name;

        m_tags.push(startTag.m_name);
        m_state = InAttributeBlock;
    }

    my_type& operator <<(const XmlEndTag& /* endTag */)
    {
        string currentTag = m_tags.pop();

        switch (m_state) {
        case InContent:
            m_stream << "</" << currentTag << ">\n";
            break;
        case InAttributeBlock:
            m_stream << " />\n";
            break;
        default:
            throw ios_base::failure("all tags closed");
        }

        if (m_tags.empty()) m_state = None;
        else m_state = InContent;
    }

    // delegate all remaining types to stream_type
    template<class T>
    my_type& operator <<(const T& value)
    {
        m_stream << value;
        return *this;
    }

    basic_string<Ch, Tr> str() const
    {
        return m_stream.str();
    }

private:
    State m_state;
    stack<string> m_tags;
    stream_type& m_stream;
};

template<class Ch, class Tr>
inline const XmlAttribute<Ch, Tr> attribute(
    const basic_string<Ch, Tr>& name,
    const basic_string<Ch, Tr>& content)
{
    return XmlAttribute<Ch, Tr>(name, content);
}

template<class Ch, class Tr>
inline const XmlContent<Ch, Tr> content(
    const basic_string<Ch, Tr>& str)
{
    return XmlContent<Ch, Tr>(str);
}

template<class Ch, class Tr>
inline const XmlStartTag<Ch, Tr> startTag(
    const basic_string<Ch, Tr>& name)
{
    return XmlStartTag<Ch, Tr>(name);
}

inline const XmlEndTag endTag()
{
    return XmlEndTag();
}

} // end namespace xml
