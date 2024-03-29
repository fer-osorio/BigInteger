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
    // -Using BigInteger(const char[], NumberBase = HEXADECIMAL) implicitly.
    BigInteger n0="FFFF""FFFFFFFFFFFFFFFF""FFFFFFFFFFFFFFFF""FFFFFFFFFFFFFFFF";
    BigInteger n1="-FFFF""FFFFFFFFFFFFFFFF""FFFFFFFFFFFFFFFF""FFFFFFFFFFFFFFFF";
    BigInteger quorem[2];
    //BigInteger n2 = 2;
    //BigInteger n3 = "FFFF";

    std::cout << "n0 =        " ; n0.printHexln();
    std::cout << "n1 =       "; n1.printHexln(); std::cout << '\n';

    std::cout << "n0 - 1   =  "; (n0 - 1).printHexln();
    std::cout << "n0 + 1   =  "; (n0 + 1).printHexln();
    // ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~^
    // Calling BigInteger(ui64) implicitly)

    std::cout << "n0 + -n1 =  "; (n0 + -n1).printHexln();
    std::cout << "n0 + n1  =  "; (n0 + n1).printHexln();
    std::cout << "n1 + n0  =  "; (n1 + n0).printHexln();
    std::cout << "n1 + -n0 = "; (n1 + -n0).printHexln(); std::cout << '\n';

    std::cout << "n0 - -n1 =  "; (n0 - -n1).printHexln();
    std::cout << "n0 - n1  =  "; (n0 - n1).printHexln();
    std::cout << "n1 - n0  = "; (n1 - n0).printHexln();
    std::cout << "-n0 - n1 =  "; (-n0 - n1).printHexln(); std::cout << '\n';
    // Let 0 <= v <= w
    // (2^w - 1)*(2^v - 1) = 2^{w+v} - 2^w - 2^v + 1
    //                     = 2^{w+v} - 2^v*(2^{w-v} + 1) + 1
    // (2^w - 1)^2 = 2^{2*w} - 2^{w+1} + 1
    std::cout << "n0*(-n1+1) =  "; (n0*(-n1+1)).printHexln();
    std::cout << "n0*n1      = "; (n0*n1).printHexln(); std::cout << '\n';

    n0.shortDivision(0x10000, quorem);
    std::cout << "n0 / 2^16 = " ; quorem[0].printHexln();
    std::cout << "n0 % 2^16 = " ; quorem[1].printHexln(); std::cout << '\n';

    if(n1 < n0) { n1.printHex(); std::cout <<  " < "; n0.printHexln(); }
    if(n1 < 0)  { n1.printHex(); std::cout << " < 0\n"; }
    if(quorem[0] < n0)  { quorem[0].printHex(); std::cout << " < "; n0.printHexln(); }

    return 0;
}

