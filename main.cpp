/* main.cpp
 *
 * Copyright 2023 Fernando Osorio
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include"BigInteger.hpp"

int main(void)
{
    BigInteger n0("FFFFFFFFFFFFFFFF""FFFFFFFFFFFFFFFF""FFFFFFFFFFFFFFFF""FFFF");
  //BigInteger n1("FFFFFFFFFFFFFFFF""FFFFFFFFFFFFFFFF""FFFFFFFFFFFFFFFF""FFFF");
    BigInteger n1(1);
    BigInteger r = n0 + n1;
    //std::cout << big << '\n';
    r.println();

    return 0;
}

