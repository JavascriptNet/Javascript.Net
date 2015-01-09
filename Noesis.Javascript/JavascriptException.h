////////////////////////////////////////////////////////////////////////////////////////////////////
// File: JavascriptException.h
// 
// Copyright 2010 Noesis Innovation Inc. All rights reserved.
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
//       notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
//       copyright notice, this list of conditions and the following
//       disclaimer in the documentation and/or other materials provided
//       with the distribution.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////////////////////////////////////////////////////////////////////////////////////////////////////

#pragma once

////////////////////////////////////////////////////////////////////////////////////////////////////

#include <v8.h>

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis { namespace Javascript {

////////////////////////////////////////////////////////////////////////////////////////////////////

using namespace v8;

////////////////////////////////////////////////////////////////////////////////////////////////////
// JSScript
////////////////////////////////////////////////////////////////////////////////////////////////////
public ref class JavascriptException: System::Exception
{
	////////////////////////////////////////////////////////////
	// Public Constructors
	////////////////////////////////////////////////////////////
internal:

	JavascriptException(TryCatch& iTryCatch);
	JavascriptException(wchar_t const *complaint);

	////////////////////////////////////////////////////////////
	// Public Methods
	////////////////////////////////////////////////////////////
public:

	virtual property System::String^ Source
	{ System::String^ get () override; }

	virtual property int Line
	{ int get(); }

	virtual property int StartColumn
	{ int get(); }

	virtual property int EndColumn
	{ int get(); }

	static System::Exception^ GetSystemException(TryCatch& iTryCatch);


	////////////////////////////////////////////////////////////
	// Private Methods
	////////////////////////////////////////////////////////////
private:

	static System::String^ GetExceptionMessage(TryCatch& iTryCatch);


	////////////////////////////////////////////////////////////
	// Data members
	////////////////////////////////////////////////////////////
private:
	
	System::String^ mSource;
	int mLine;
	int mStartColumn, mEndColumn;  // on mLine
};

////////////////////////////////////////////////////////////////////////////////////////////////////

} } // namespace Noesis::Javascript

////////////////////////////////////////////////////////////////////////////////////////////////////