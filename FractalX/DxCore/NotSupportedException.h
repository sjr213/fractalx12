#pragma once

namespace DxCore
{
	class CNotSupportedException : public std::exception
	{
	public:
		CNotSupportedException(char const* const _Message) throw();

		CNotSupportedException(const std::string& msg) throw();

		CNotSupportedException(const CNotSupportedException& ex) throw();

		virtual ~CNotSupportedException();
	};
}