#ifndef FileSizeDefine_h__
#define FileSizeDefine_h__
#include "qglobal.h"


#pragma pack(push,_CRT_PACKING)
#pragma warning(push,3)
#pragma push_macro("new")
#undef new
#pragma warning(disable: 4455)
constexpr qint64 MBBytes = 1024 * 1024;
inline constexpr qint64 operator "" MB(unsigned long long val)
{	// return integral MBs
	return val * MBBytes;
}

inline constexpr long double operator "" MB(long double val)
{	// return floating-point MBs
	return val * MBBytes;
}
constexpr qint64 GBBytes = 1024 * 1024 * 1024;
inline constexpr qint64 operator "" GB(unsigned long long val)
{	// return integral GBs
	return val * GBBytes;
}

inline constexpr long double operator "" GB(long double val)
{	// return floating-point GBs
	return val * GBBytes;
}
#pragma pop_macro("new")
#pragma warning(pop)
#pragma pack(pop)

#endif // FileSizeDefine_h__
