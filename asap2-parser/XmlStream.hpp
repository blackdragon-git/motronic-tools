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

#include <cassert>
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
    const basic_string<Ch, Tr> m_name;
//    const basic_string<Ch, Tr> m_content;

    XmlAttribute(
        const basic_string<Ch, Tr>& name) :
//        const basic_string<Ch, Tr>& content) :
        m_name(name)
//        m_content(content)
    { }

    // copy constructor
    XmlAttribute(const XmlAttribute& attribute) :
        m_name(attribute.m_name)
//        m_content(attribute.m_content)
    { }
};

template<class Ch, class Tr = char_traits<Ch> >
struct XmlContent : public XmlElement
{
    const basic_string<Ch, Tr> m_content;

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
    const basic_string<Ch, Tr> m_name;

    XmlStartTag(const basic_string<Ch, Tr> name) :
        m_name(name)
    { }

    // copy constructor
    XmlStartTag(const XmlStartTag& startTag) :
        m_name(startTag.m_name)
    { }
};

struct XmlEndTag : public XmlElement
{
    unsigned int m_count;

    XmlEndTag(int count) :
        m_count(count)
    { }

    // copy constructor
    XmlEndTag(const XmlEndTag& endTag) :
        m_count(endTag.m_count)
    { }
};

template<class Ch, class Tr = char_traits<Ch> >
class XmlStream
{
public:
    typedef XmlStream my_type;
    typedef basic_stringstream<Ch, Tr> stream_type;
    typedef stack<basic_string<Ch, Tr> > stack_type;

    enum State { InContent, InAttribute, InAttributeBlock, TagEnd, None };

    XmlStream(stream_type& stream) :
        m_state(None),
        m_tags(),
        m_stream(stream)
    { }

    my_type& operator <<(const XmlAttribute<Ch, Tr>& attribute)
    {
        tryToCloseAttribute();
        if (m_state != InAttributeBlock) {
            throw ios_base::failure("not in attribute block");
        }

        m_stream << ' ' << attribute.m_name << "=\"";
//                 << attribute.m_content << '"';
        m_state = InAttribute;

        return *this;
    }

    my_type& operator <<(const XmlContent<Ch, Tr>& content)
    {
        tryToCloseAttribute();
        if (m_state == InAttributeBlock) {
            m_stream << '>';
        }

        m_stream << content.m_content;
        m_state = InContent;

        return *this;
    }

    my_type& operator <<(const XmlStartTag<Ch, Tr>& startTag)
    {
        tryToCloseAttribute();
        if (m_state == InAttributeBlock) {
            m_stream << ">\n";
        }

        incise();
        m_stream << '<' << startTag.m_name;

        m_tags.push(startTag.m_name);
        m_state = InAttributeBlock;

        return *this;
    }

    my_type& operator <<(const XmlEndTag& endTag)
    {
        closeTags(endTag.m_count);
        return *this;
    }

    // handle function pointers
    my_type& operator <<(my_type& (*f)(my_type&))
    {
        return f(*this);
    }

    // delegate all remaining types to stream_type
    template<class T>
    my_type& operator <<(const T& value)
    {
        m_stream << value;
        return *this;
    }

    void closeTags(unsigned int count = 1)
    {
        assert(m_tags.size() >= count);

        tryToCloseAttribute();
        for (int i = count; i > 0; --i) {
            basic_string<Ch, Tr>& currentTag = m_tags.top();

            switch (m_state) {
            case TagEnd:
                incise(-1);
            case InContent:
                m_stream << "</" << currentTag << ">\n";
                break;
            case InAttributeBlock:
                m_stream << " />\n";
                break;
            default:
                throw ios_base::failure("all tags closed");
            }

            m_tags.pop();
        }

        if (m_tags.empty()) m_state = None;
        else m_state = TagEnd;//InContent;
    }

    basic_string<Ch, Tr> str() const
    {
        return m_stream.str();
    }

    void enterContent()
    {
        if (m_state != InAttributeBlock) {
            throw ios_base::failure("not in attribute block");
        }

        m_state = InContent;
        m_stream << '>';
    }

private:
    State m_state;
    stack_type m_tags;
    stream_type& m_stream;

    bool tryToCloseAttribute()
    {
        if (m_state == InAttribute) {
            m_stream << '"';
            m_state = InAttributeBlock;
            return true;
        }

        return false;
    }

    void incise(int offset = 0)
    {
        int i = m_tags.size() + offset;
        for (; i != 0; --i) m_stream << "  ";
    }
};

template<class Ch, class Tr>
inline const XmlAttribute<Ch, Tr> attribute(
    const basic_string<Ch, Tr>& name)
//    const basic_string<Ch, Tr>& content)
{
    return XmlAttribute<Ch, Tr>(name);//, content);
}

template<class Ch>
inline const XmlAttribute<Ch> attribute(
    const Ch* name)
//    const Ch* content)
{
    return XmlAttribute<Ch>(basic_string<Ch>(name));//, basic_string<Ch>(content));
}

template<class Ch, class Tr>
inline const XmlContent<Ch, Tr> content(
    const basic_string<Ch, Tr>& str)
{
    return XmlContent<Ch, Tr>(str);
}

template<class Ch>
inline const XmlContent<Ch> content(
    const Ch* str)
{
    return XmlContent<Ch>(basic_string<Ch>(str));
}

template<class Ch, class Tr>
XmlStream<Ch, Tr>& content(XmlStream<Ch, Tr>& stream)
{
    stream.enterContent();
    return stream;
}

template<class Ch, class Tr>
inline const XmlStartTag<Ch, Tr> startTag(
    const basic_string<Ch, Tr>& name)
{
    return XmlStartTag<Ch, Tr>(name);
}

template<class Ch>
inline const XmlStartTag<Ch> startTag(
    const Ch* name)
{
    return XmlStartTag<Ch>(basic_string<Ch>(name));
}

inline const XmlEndTag endTag(int count = 1)
{
    return XmlEndTag(count);
}

template<class Ch, class Tr>
XmlStream<Ch, Tr>& endTag(XmlStream<Ch, Tr>& stream)
{
    stream.closeTags(1);
    return stream;
}

} // end namespace xml
