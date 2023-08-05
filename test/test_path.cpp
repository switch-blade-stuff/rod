/*
 * Created by switch_blade on 2023-08-04.
 */

#include <rod/path.hpp>

#include "common.hpp"

int main()
{
	rod::fs::path path = "//host/dir/name.ext";

	/* Win32 supports UNC syntax */
#ifdef ROD_WIN32
	TEST_ASSERT(path.root_path() == "//host/");
	TEST_ASSERT(path.root_name() == "//host");
#else
	TEST_ASSERT(path.root_path() == "/");
	TEST_ASSERT(path.root_name() == "");
#endif

	TEST_ASSERT(path.root_directory() == "/");
	TEST_ASSERT(path.filename() == "name.ext");
	TEST_ASSERT(path.extension() == ".ext");
	TEST_ASSERT(path.stem() == "name");

	path = (path / "../subdir/./name.ext1").lexically_normal();
	TEST_ASSERT(path == rod::fs::path("//host/dir/subdir/name.ext1").lexically_normal());

	path += ".ext2";
	TEST_ASSERT(path.extension() == ".ext2");
	path.replace_extension();
	TEST_ASSERT(path.extension() == ".ext1");
	path.replace_extension("ext");
	TEST_ASSERT(path.extension() == ".ext");
	path.replace_extension();
	TEST_ASSERT(path.extension() == "");
}
