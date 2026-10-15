/*
    Stride is licensed under the terms of the GNU Affero General Public License version 3.

    Copyright (C) 2017-2026. The Regents of the University of California.
    All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.

    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.

    Authors: Andres Cabrera and Joseph Tilbian
*/

#ifndef BUILDTESTER_HPP
#define BUILDTESTER_HPP

#include <string>


class BuildTester
{
public:
    BuildTester(std::string strideRoot = "/home/andres/Documents/src/Stride/Stride/strideroot");
    bool test(std::string filename, std::string expectedResultFile, bool tolerant = false);
    
private:
    std::string m_StrideRoot;
};

#endif // BUILDTESTER_HPP
