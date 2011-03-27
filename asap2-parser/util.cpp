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

#include "util.h"

#include "node.h"
#include "parser.hpp"

void getDataTypeInfo(int type, short* sizeInBits, bool* isSigned)
{
    switch (type) {
    case TUBYTE:
        *sizeInBits = 8;
        *isSigned = false;
        return;
    case TSBYTE:
        *sizeInBits = 8;
        *isSigned = true;
        return;
    case TUWORD:
        *sizeInBits = 16;
        *isSigned = false;
        return;
    case TSWORD:
        *sizeInBits = 16;
        *isSigned = true;
        return;
    case TULONG:
        *sizeInBits = 32;
        *isSigned = false;
        return;
    case TSLONG:
        *sizeInBits = 32;
        *isSigned = true;
        return;
    case TFLOAT32: // TODO
        *sizeInBits = 32;
        *isSigned = true;
        return;
    }
}
