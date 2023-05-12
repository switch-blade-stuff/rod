/*
 * Created by switchblade on 2023-05-02.
 */

#include <stdexcept>

#ifdef __GNUC__

#include <typeinfo>
#include <cstdio>
#include <atomic>

#include <execinfo.h>
#include <unwind.h>
#include <dlfcn.h>

_Unwind_Reason_Code helper(struct _Unwind_Context *ctx, void *)
{
	void *p = reinterpret_cast<void *>(_Unwind_GetIP(ctx));
	Dl_info info;
	if (dladdr(p, &info) && info.dli_saddr)
	{
		auto d = reinterpret_cast<long>(p) - reinterpret_cast<long>(info.dli_saddr);
		fprintf(stderr, "%p %s+0x%lx\n", p, info.dli_sname, d);
		fflush(stderr);
	}
	return _URC_NO_REASON;
}

extern "C" [[noreturn]] void __cxa_throw(void *data, void *rtti, void (*dtor)(void *))
{
	const auto real_cxa_throw = (void (*)(void *, std::type_info *, void (*)(void *))) dlsym(RTLD_NEXT, "__cxa_throw");
	auto *type = static_cast<std::type_info *>(rtti);

	_Unwind_Backtrace(helper, 0);
	real_cxa_throw(data, type, dtor);
	__builtin_unreachable();
}

#endif

void __attribute__(( noinline )) zoo()
{
	std::puts("ping!");
	throw 1;
}

__attribute__((__noinline__)) void bar(void (*f)()) { f(); }
__attribute__((__noinline__)) void foo()
{
	try
	{
		bar(&zoo);
	}
	catch (int)
	{
		std::puts("pong!");
	}
}

int main()
{
	foo();
	return 0;
}
