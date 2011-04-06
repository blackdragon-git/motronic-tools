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
#include <fstream>

#include "owner_ptr.hpp"
#include "node.h"

using namespace std;

class GenericNode : public Node
{
public:
    virtual ~GenericNode() { }

    friend ostream& operator<<(ostream& stream, GenericNode& node);
    friend istream& operator>>(istream& stream, GenericNode& node);
};

ostream& operator<<(ostream& stream, GenericNode& node)
{
//    stream <<
    return stream;
}

istream& operator>>(istream& stream, GenericNode& node)
{
//    stream >>
    return stream;
}

class GCharacteristic : public GenericNode
{
    // TODO
};

class GMap : public GCharacteristic
{
    // TODO
};

class GCurve : public GCharacteristic
{
    // TODO
};

class GValue : public GenericNode
{
    // TODO
};
