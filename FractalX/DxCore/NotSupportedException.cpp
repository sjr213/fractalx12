#include "stdafx.h"
#include "NotSupportedException.h"

namespace DxCore
{
	CNotSupportedException::CNotSupportedException(char const* const _Message) throw()
		: std::exception(_Message)
	{}

	CNotSupportedException::CNotSupportedException(const std::string& msg) throw()
		: std::exception(msg.c_str())
	{}

	CNotSupportedException::CNotSupportedException(const CNotSupportedException& ex) throw()
		: std::exception(ex.what())
	{}

	CNotSupportedException::~CNotSupportedException()
	{}
}