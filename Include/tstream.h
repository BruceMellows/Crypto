// NO LICENSE
// ==========
// There is no copyright, you can use and abuse this source without limit.
// There is no warranty, you are responsible for the consequences of your use of this source.
// There is no burden, you do not need to acknowledge this source in your use of this source.

#pragma once

#include <iostream>

#ifdef  _UNICODE
#define tcout wcout
#else /* !_UNICODE */
#define tcout cout
#endif /* _UNICODE */
