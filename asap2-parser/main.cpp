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

#include <iostream>
#include <cstdio>

#include <boost/foreach.hpp>

#include "stack.hpp"
#include "node.h"
#include "xdfGen.h"

using namespace std;

extern int yyparse();
extern NProject* projectBlock;
extern std::vector<std::string*> value_tokens;
extern ext::stack<Node*> nodes;

int main(int argc, char* argv[])
{
    int result = yyparse();
    BOOST_FOREACH (std::vector<std::string*>::value_type i, value_tokens) {
        delete i;
    }

    // delete our remaining nodes
    BOOST_FOREACH (ext::stack<Node*>::value_type i, nodes) {
        if (!i->hasParent())
            delete i;
    }

    if (result) {
        std::cerr << "Failed to parse input stream!" << std::endl;
        return result;
    }

    std::cout << projectBlock << endl;
    if (projectBlock == NULL) {
        return -1;
    }

    //	getchar();

    XdfGen generator(projectBlock->m_module.ref(), -0x800000);

    const CharacteristicHashMap& characteristics = projectBlock->m_module->characteristics;
    BOOST_FOREACH (CharacteristicHashMap::value_type i, characteristics) {
        NStatement* current = i.second;
        assert(current != NULL);
        current->accept(generator);
    }
    generator.epilogue();

    delete projectBlock; // this will delete our whole tree
    projectBlock = NULL;

    return 0;
}
