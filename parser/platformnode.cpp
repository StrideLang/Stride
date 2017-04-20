/*
    Stride is licensed under the terms of the 3-clause BSD license.

    Copyright (C) 2017. The Regents of the University of California.
    All rights reserved.
    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright
        notice, this list of conditions and the following disclaimer in the
        documentation and/or other materials provided with the distribution.

        Neither the name of the copyright holder nor the names of its
        contributors may be used to endorse or promote products derived from
        this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    Authors: Andres Cabrera and Joseph Tilbian
*/

#include <sstream>
#include <iostream>
#include <vector>
#include <cstdlib>

#include "platformnode.h"

PlatformNode::PlatformNode(string platformName, double version, const char *filename, int line, string hwPlatform, double hwVersion) :
    AST(AST::Platform, filename, line)
{
    m_platformName = platformName;
    m_version = version;
    m_targetPlatform.name = hwPlatform;
    m_targetPlatform.version = hwVersion;
}

PlatformNode::~PlatformNode()
{

}

double PlatformNode::version() const
{
    return m_version;
}

void PlatformNode::setVersion(double version)
{
    m_version = version;
}

AST *PlatformNode::deepCopy()
{
    AST *newnode = new PlatformNode(m_platformName, m_version, m_filename.data() , m_line, m_targetPlatform.name, m_targetPlatform.version);
    vector<AST *> children = getChildren();
    for (unsigned int i = 0; i < children.size(); i++) {
        newnode->addChild(children.at(i)->deepCopy());
    }
    return newnode;
}
string PlatformNode::hwPlatform() const
{
    return m_targetPlatform.name;
}

void PlatformNode::setHwPlatform(const string &hwPlatform)
{
    m_targetPlatform.name = hwPlatform;
}
double PlatformNode::hwVersion() const
{
    return m_targetPlatform.version;
}

void PlatformNode::setHwVersion(double hwVersion)
{
    m_targetPlatform.version = hwVersion;
}

string PlatformNode::platformName() const
{
    return m_platformName;
}

void PlatformNode::setPlatformName(const string &platformName)
{
    m_platformName = platformName;
}

