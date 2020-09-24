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

#include "langerror.h"

#include <sstream>

LangError::LangError() {}

std::string LangError::getErrorText() {
  std::string errorText = "In file " + filename + ":\n    ";
  std::stringstream lineNumString;
  lineNumString << lineNumber;
  errorText += "Line " + lineNumString.str() + " : ";
  switch (type) {
  case Syntax:
    errorText += "Syntax Error: Unexpected character '" + errorTokens[0] + "'";
    break;
  case UnknownType:
    errorText +=
        "Unknown Type Error. Type '" + errorTokens[0] + "' not recognized.";
    break;
  case InvalidType:
    errorText += "Invalid Type: '" + errorTokens[0] + "'";
    break;
  case InvalidPort:
    errorText += "Invalid Port '" + errorTokens[1] + "' for type '" +
                 errorTokens[0] + "'";
    break;
  case InvalidPortType:
    errorText += "Invalid port type Error. Port '" + errorTokens[1] +
                 "' in Block '" + errorTokens[0] + "' expects '" +
                 errorTokens[3] + "'" + "' given '" + errorTokens[2] + "'";
    break;
  case InvalidIndexType:
    errorText += "Invalid index type.";
    break;
  case BundleSizeMismatch:
    errorText += "Bundle Size Mismatch Error";
    break;
  case ArrayIndexOutOfRange:
    errorText += "Array Index out of Range Error";
    break;
  case DuplicateSymbol:
    errorText += "Duplicate Symbol Error. '" + errorTokens[0] +
                 "' redefined in " + errorTokens[1] + ":" + errorTokens[2] + "";
    break;
  case InconsistentList:
    errorText += "Inconsistent List Error";
    break;
  case StreamMemberSizeMismatch:
    errorText += "Stream size mismatch. '" + errorTokens[1] + "' provides " +
                 errorTokens[0] + "outs , " + errorTokens[2] + " expected.";
    break;
  case UndeclaredSymbol:
    errorText += "Undeclared Symbol '" + errorTokens[0] + "'";
    break;
  case SystemError:
    errorText += "Undeclared Symbol '" + errorTokens[0] + "'";
    break;
  case UnknownPlatform:
    errorText += "Unknown Platform '" + errorTokens[0] + "'";
    break;
  case UnresolvedRate:
    errorText += "Rate Unresolved for '" + errorTokens[0] + "'";
    break;
  case None:
    errorText += "Unknown error";
    break;
  default:
    break;
  }
  return errorText;
}
