////////////////////////////////////////////////////////////////////////////////////////////////////
// File: SystemInterop.cpp
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

#include "SystemInterop.h"
#include "JavascriptInterop.h"
#include "JavascriptContext.h"
#include "JavascriptExternal.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

namespace Noesis {

////////////////////////////////////////////////////////////////////////////////////////////////////

// Returns null if no conversion is possible.
System::Object^
SystemInterop::ConvertToType(System::Object^ iValue, System::Type^ iType)
{
	if (iValue != nullptr)
	{
		System::Type^ type = iValue->GetType();
        System::Type^ potentiallyUnderlyingType = System::Nullable::GetUnderlyingType(iType);

        if (potentiallyUnderlyingType != nullptr)
            iType = potentiallyUnderlyingType;

		if (type == iType || iType->IsAssignableFrom(type) || iType == System::Object::typeid)
			return iValue;

		if (iType->IsArray)
			return ConvertArray(iValue, iType);

		if (iType == System::Boolean::typeid)
			return ConvertToBoolean(iValue);		
		if (iType == System::Int16::typeid)
			return ConvertToInt16(iValue);
		else if (iType == System::Int32::typeid)
			return ConvertToInt32(iValue);
		else if (iType == System::Single::typeid)
			return ConvertToSingle(iValue);
		else if (iType == System::Double::typeid)
			return ConvertToDouble(iValue);
		else if (iType == System::Decimal::typeid)
			return ConvertToDecimal(iValue);
		else if (iType == System::String::typeid)
			return ConvertToString(iValue);
		else if (iType->IsEnum)
			return ConvertToEnum(iType, iValue);
	}

	return nullptr;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

bool
SystemInterop::ConvertToBoolean(System::Object^ iValue)
{
	if (iValue != nullptr)
	{
		System::Type^ type = iValue->GetType();

		if (type == System::Boolean::typeid)
			return (bool) iValue;
		else if (type == System::Int16::typeid)
			return ((short) iValue) != 0;
		else if (type == System::Int32::typeid)
			return ((int) iValue) != 0;
		else if (type == System::Single::typeid)
			return ((float) iValue) != 0.0f;
		else if (type == System::Double::typeid)
			return ((double) iValue) != 0.0;
		else if (type == System::Decimal::typeid)
			return (double)((System::Decimal)iValue) != 0.0;
		else if (type == System::String::typeid)
		{
			bool ret;
			if (System::Boolean::TryParse((System::String^) iValue, ret))
				return ret;
		}

		return true;
	}

	return false;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
SystemInterop::ConvertToEnum(System::Type^ enum_type, System::Object^ iValue)
{
	if (iValue == nullptr) {
		return System::Enum::ToObject(enum_type, 0);
	} else {
		System::Type^ type = iValue->GetType();

		int numeric_value;
		if (type == System::String::typeid) {
            try {
                return System::Enum::Parse(enum_type, (System::String^)iValue);
            } catch (System::ArgumentException ^) {
                return nullptr;
            }
		} else if (type == System::Int16::typeid
			|| type == System::Int32::typeid
			|| type == System::Single::typeid
			|| type == System::Double::typeid
			|| type == System::Decimal::typeid) {
			numeric_value = System::Convert::ToInt32(iValue);
			return System::Enum::ToObject(enum_type, numeric_value);
		} else {
			return System::Enum::ToObject(enum_type, 0);
		}
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////

short
SystemInterop::ConvertToInt16(System::Object^ iValue)
{
	if (iValue != nullptr)
	{
		System::Type^ type = iValue->GetType();

		if (type == System::Boolean::typeid)
			return ((bool) iValue) ? -1 : 0;
		else if (type == System::Int16::typeid)
			return (short) iValue;
		else if (type == System::Int32::typeid)
			return (short) iValue;
		else if (type == System::Single::typeid)
			return (short) ((float) iValue);
		else if (type == System::Double::typeid)
			return (short) ((double) iValue);
		else if (type == System::Decimal::typeid)
			return (short)((System::Decimal)iValue);
		else if (type == System::String::typeid)
		{
			short ret;
			if (System::Int16::TryParse((System::String^) iValue, ret))
				return ret;
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

int
SystemInterop::ConvertToInt32(System::Object^ iValue)
{
	if (iValue != nullptr)
	{
		System::Type^ type = iValue->GetType();

		if (type == System::Boolean::typeid)
			return ((bool) iValue) ? -1 : 0;
		else if (type == System::Int16::typeid)
			return (int) iValue;
		else if (type == System::Int32::typeid)
			return (int) iValue;
		else if (type == System::Single::typeid)
			return (int) ((float) iValue);
		else if (type == System::Double::typeid)
			return (int) ((double) iValue);
		else if (type == System::Decimal::typeid)
			return (int)((System::Decimal)iValue);
		else if (type == System::String::typeid)
		{
			int ret;
			if (System::Int32::TryParse((System::String^) iValue, ret))
				return ret;
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

float
SystemInterop::ConvertToSingle(System::Object^ iValue)
{
	if (iValue != nullptr)
	{
		System::Type^ type = iValue->GetType();

		if (type == System::Boolean::typeid)
			return ((bool) iValue) ? -1.0f : 0.0f;
		else if (type == System::Int16::typeid)
			return (float) ((short) iValue);
		else if (type == System::Int32::typeid)
			return (float) ((int) iValue);
		else if (type == System::Single::typeid)
			return (float) iValue;
		else if (type == System::Double::typeid)
			return (float) ((double) iValue);
		else if (type == System::Decimal::typeid)
			return (float)((System::Decimal)iValue);
		else if (type == System::String::typeid)
		{
			float ret;
			if (System::Single::TryParse((System::String^) iValue, ret))
				return ret;
		}
	}

	return 0.0f;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
SystemInterop::ConvertToDouble(System::Object^ iValue)
{
	if (iValue != nullptr)
	{
		System::Type^ type = iValue->GetType();

		if (type == System::Boolean::typeid)
			return ((bool)iValue) ? -1.0 : 0.0;
		else if (type == System::Int16::typeid)
			return (double)((short)iValue);
		else if (type == System::Int32::typeid)
			return (double)((int)iValue);
		else if (type == System::Single::typeid)
			return (double)((float)iValue);
		else if (type == System::Double::typeid)
			return (double)iValue;
		else if (type == System::Decimal::typeid)
			return (double)((System::Decimal)iValue);
		else if (type == System::String::typeid)
		{
			double ret;
			if (System::Double::TryParse((System::String^) iValue, ret))
				return ret;
		}
	}

	return 0.0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Decimal
SystemInterop::ConvertToDecimal(System::Object^ iValue)
{
	if (iValue != nullptr)
	{
		System::Type^ type = iValue->GetType();

		if (type == System::Boolean::typeid)
			return ((bool)iValue) ? -1 : 0;
		else if (type == System::Int16::typeid)
			return (System::Decimal)((short)iValue);
		else if (type == System::Int32::typeid)
			return (System::Decimal)((int)iValue);
		else if (type == System::Single::typeid)
			return (System::Decimal)((float)iValue);
		else if (type == System::Double::typeid)
			return (System::Decimal)((double)iValue);
		else if (type == System::Double::typeid)
			return (System::Decimal)iValue;
		else if (type == System::String::typeid)
		{
			System::Decimal ret;
			if (System::Decimal::TryParse((System::String^) iValue, ret))
				return ret;
		}
	}

	return 0;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::String^
SystemInterop::ConvertToString(System::Object^ iValue)
{
	if (iValue != nullptr)
		return iValue->ToString();

	return gcnew System::String("");
}

////////////////////////////////////////////////////////////////////////////////////////////////////

uint16_t*
SystemInterop::ConvertFromSystemString(System::String^ iString)
{
	System::IntPtr ptr = System::Runtime::InteropServices::Marshal::StringToHGlobalUni(iString);
	uint16_t* ret = NULL;

	if (ptr != System::IntPtr::Zero)
	{
		ret = (uint16_t*)ptr.ToPointer();
	//	System::Runtime::InteropServices::Marshal::FreeHGlobal(ptr);
	}

	return ret;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::String^
SystemInterop::ConvertToSystemString(std::string iString)
{
	return gcnew System::String(iString.c_str());
}

////////////////////////////////////////////////////////////////////////////////////////////////////

double
SystemInterop::ConvertFromSystemDateTime(System::DateTime^ iDateTime) 
{
    System::DateTime^ startDate = gcnew System::DateTime(1970, 1, 1, 0, 0, 0, 0, System::DateTimeKind::Utc);
	System::TimeSpan^ timespan = iDateTime->ToUniversalTime() - *startDate;

	return timespan->TotalMilliseconds;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

System::Object^
SystemInterop::ConvertArray(System::Object^ iValue, System::Type^ iType)
{
	if (!iValue->GetType()->IsArray || !iType->IsArray)
		return nullptr;

	System::Type^ itemType = iType->GetElementType();
	System::Array^ source = (System::Array^) iValue;
	System::Array^ result = System::Array::CreateInstance(itemType, source->Length);
	
	for (int i = 0; i < source->Length; i++)
	{
		System::Object^ item = source->GetValue(i);
		result->SetValue(ConvertToType(item, itemType), i);
	}

	return result;
}

////////////////////////////////////////////////////////////////////////////////////////////////////

} // Namepspace Noesis

////////////////////////////////////////////////////////////////////////////////////////////////////