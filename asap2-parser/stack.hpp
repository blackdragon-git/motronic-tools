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

#include <stdexcept>
#include <deque>

namespace ext {

template<class T>
class stack : public std::deque<T>
{
public:
    typedef          std::deque<T>                  base_type;
    typedef typename base_type::value_type          value_type;
    typedef typename base_type::reference           reference;
    typedef typename base_type::const_reference     const_reference;
    typedef typename base_type::size_type           size_type;

    reference atBack(size_type displacement)
    {
        size_type size = base_type::size();
        if (size < displacement)
            throw std::out_of_range("displacement");

        return base_type::at(size - displacement);
    }

    void discardRange(size_type range)
    {
        typename base_type::iterator end = base_type::end();
        base_type::erase(end - range, end);
    }
};

}
