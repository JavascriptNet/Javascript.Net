#pragma once

#include <v8.h>
using namespace v8;

namespace Noesis {
    namespace Javascript {
        public ref class JavascriptStackFrame
        {
        public:
            property int LineNumber {
                public: int get() { return mLineNumber; }
                internal: void set(int value) { mLineNumber = value; }
            }

            property int Column {
                public: int get() { return mColumn; }
                internal: void set(int value) { mColumn = value; }
            }

            property int ScriptId {
                public: int get() { return mScriptId; }
                internal: void set(int value) { mScriptId = value; }
            }

            property System::String^ ScriptName {
                public: System::String^ get() { return mScriptName; }
                internal: void set(System::String^ value) { mScriptName = value; }
            }

            property System::String^ ScriptNameOrSourceURL {
                public: System::String^ get() { return mScriptNameOrSourceURL; }
                internal: void set(System::String^ value) { mScriptNameOrSourceURL = value; }
            }

            property System::String^ FunctionName {
                public: System::String^ get() { return mFunctionName; }
                internal: void set(System::String^ value) { mFunctionName = value; }
            }

            property bool IsEval {
                public: bool get() { return mIsEval; }
                internal: void set(bool value) { mIsEval = value; }
            }

            property bool IsConstructor {
                public: bool get() { return mIsConstructor; }
                internal: void set(bool value) { mIsConstructor = value; }
            }

            property bool IsWasm {
                public: bool get() { return mIsWasm; }
                internal: void set(bool value) { mIsWasm = value; }
            }

        private:
            int mLineNumber;
            int mColumn;
            int mScriptId;
            System::String^ mScriptName;
            System::String^ mScriptNameOrSourceURL;
            System::String^ mFunctionName;
            bool mIsEval;
            bool mIsConstructor;
            bool mIsWasm;
        };
    }
}