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
#include <stdexcept>
/*
inline std::string getContent(const std::string& base)
{
	if (base.size() < 3)
		throw std::out_of_range ("const std::string& base");

	return base.substr(1, base.size()-1);
}
*/
inline std::string getAddressSubstr(const std::string& base)
{
        if (base.size() < 3) //5)
		throw std::out_of_range ("const std::string& base");

        return base.substr(/*3*/2);//, base.size()-1);
}

void getDataTypeInfo(int type, short* sizeInBits, bool* isSigned);

template<class T>
void deleteAndClear(T& container)
{
    for (typename T::iterator it = container.begin();
         it != container.end(); ++it ) {
        delete *it;
    }

    container.clear();
}
