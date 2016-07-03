/*  ===========================================================================
*
*   This file is part of HISE.
*   Copyright 2016 Christoph Hart
*
*   HISE is free software: you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation, either version 3 of the License, or
*   (at your option) any later version.
*
*   HISE is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with HISE.  If not, see <http://www.gnu.org/licenses/>.
*
*   Commercial licences for using HISE in an closed source project are
*   available on request. Please visit the project's website to get more
*   information about commercial licencing:
*
*   http://www.hartinstruments.net/hise/
*
*   HISE is based on the JUCE library,
*   which also must be licenced for commercial applications:
*
*   http://www.juce.com
*
*   ===========================================================================
*/

/* HI Module */

#include "hi_scripting.h"

#include "scripting/HiseJavascriptEngine.cpp"
#include "scripting/JavascriptEngineExpressions.cpp"
#include "scripting/JavascriptEngineParser.cpp"
#include "scripting/JavascriptEngineObjects.cpp"

#include "scripting/api/XmlApi.cpp"
#include "scripting/api/ScriptingApi.cpp"
#include "scripting/api/ScriptingApiWrappers.cpp"


#include "scripting/ScriptProcessor.cpp"
#include "scripting/HardcodedScriptProcessor.cpp"

#include "scripting/components/ScriptingCodeEditor.cpp"

#include "scripting/api/ScriptComponentWrappers.cpp"
#include "scripting/components/ScriptingContentComponent.cpp"

#if USE_BACKEND

#include "scripting/components/ScriptingEditor.cpp"

#endif 