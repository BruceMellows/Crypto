// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include <string>

#ifdef  _UNICODE
#include <Windows.h>
#define tstring wstring
#else /* !_UNICODE */
#define tstring string
#endif /* _UNICODE */

namespace std
{
	inline std::tstring ToTString(unsigned long n)
	{
		TCHAR buffer[32];
#ifdef  _UNICODE
		wsprintf(buffer, L"%d", n);
#else /* !_UNICODE */
		sprintf(buffer, "%d", n);
#endif /* _UNICODE */
		return std::tstring(buffer);
	}
}
