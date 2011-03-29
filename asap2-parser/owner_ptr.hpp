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

#include <boost/assert.hpp>
#include <boost/checked_delete.hpp>

template<class T, class P>
class owner_ptr
{
private:
    T * m_p;

    // noncopyable
    owner_ptr(const owner_ptr &);
    owner_ptr & operator=(const owner_ptr &);

public:
    typedef T element_type;

    explicit owner_ptr(T * p, const P * parent) : m_p(p)
    {
        BOOST_ASSERT( m_p != 0 ); // the pointer is guaranteed to be valid
        p->setParent(parent);
    }

    ~owner_ptr()
    {
        boost::checked_delete(m_p);
    }

    T & operator*() const // never throws
    {
        return *m_p;
    }

    T * operator->() const // never throws
    {
        return m_p;
    }

    T * get() const // never throws
    {
        return m_p;
    }
};
