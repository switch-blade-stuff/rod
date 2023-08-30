/*
 * Created by switch_blade on 2023-08-04.
 */

#include <rod/path.hpp>

#include "common.hpp"

int main()
{
	 auto view = rod::path_view("//root/dir/name.ext");
#ifdef ROD_WIN32
	TEST_ASSERT(view.compare(L"//root\\dir\\\\\\\\name.ext") == 0);
	TEST_ASSERT(view.root_path() == rod::path_view("//root/"));
	TEST_ASSERT(view.root_name() == rod::path_view("//root"));
#else
	TEST_ASSERT(view.compare(L"//root/dir///name.ext") == 0);
	TEST_ASSERT(view.root_path() == rod::path_view("/"));
	TEST_ASSERT(view.root_name() == rod::path_view(""));
#endif
	TEST_ASSERT(view.root_directory() == rod::path_view("/"));
	TEST_ASSERT(view.relative_path() == rod::path_view("dir/name.ext"));
	TEST_ASSERT(view.parent_path() == rod::path_view("//root/dir"));
	TEST_ASSERT(view.filename() == rod::path_view("name.ext"));
	TEST_ASSERT(view.extension() == rod::path_view(".ext"));
	TEST_ASSERT(view.stem() == rod::path_view("name"));

	TEST_ASSERT(view.remove_extension() == rod::path_view("//root/dir/name"));
	TEST_ASSERT(view.remove_filename() == rod::path_view("//root/dir/"));

	const auto rendered_wchar = view.render_null_terminated<wchar_t>();
	const auto rendered_char8 = view.render_null_terminated<char8_t>();
	const auto rendered_char16 = view.render_null_terminated<char16_t>();
	const auto rendered_char32 = view.render_null_terminated<char32_t>();

	TEST_ASSERT(std::wstring_view(L"//root/dir/name.ext") == rendered_wchar.c_str());
	TEST_ASSERT(std::u8string_view(u8"//root/dir/name.ext") == rendered_char8.c_str());
	TEST_ASSERT(std::u16string_view(u"//root/dir/name.ext") == rendered_char16.c_str());
	TEST_ASSERT(std::u32string_view(U"//root/dir/name.ext") == rendered_char32.c_str());

	auto path = rod::path("//root/dir/name.ext", std::locale());
	TEST_ASSERT(rod::path(view) == path);

	/* Win32 supports UNC syntax */
#ifdef ROD_WIN32
	TEST_ASSERT(path.root_path() == "//root/");
	TEST_ASSERT(path.root_name() == "//root");
#else
	TEST_ASSERT(path.root_path() == "/");
	TEST_ASSERT(path.root_name() == "");
#endif
	TEST_ASSERT(path.root_directory() == "/");
	TEST_ASSERT(path.relative_path() == "dir/name.ext");
	TEST_ASSERT(path.parent_path() == "//root/dir");
	TEST_ASSERT(path.filename() == "name.ext");
	TEST_ASSERT(path.extension() == ".ext");
	TEST_ASSERT(path.stem() == "name");

	path = (path / "../subdir/./name.ext1").lexically_normal();
	TEST_ASSERT(path == rod::path("//root/dir/subdir/name.ext1").lexically_normal());

	path += ".ext2";
	TEST_ASSERT(path.extension() == ".ext2");
	path.remove_extension();
	TEST_ASSERT(path.extension() == ".ext1");
	path.replace_extension("ext");
	TEST_ASSERT(path.extension() == ".ext");
	path.remove_extension();
	TEST_ASSERT(path.extension() == "");

	path = "name.ext";
	path.replace_stem("name2");
	TEST_ASSERT(path.filename() == "name2.ext");
	path.replace_filename("name3");
	TEST_ASSERT(path.filename() == "name3");
	path.replace_extension(".ext");
	TEST_ASSERT(path.filename() == "name3.ext");
	path.remove_stem();
	TEST_ASSERT(path.filename() == ".ext");

	TEST_ASSERT(rod::path("/a/d").lexically_relative("/a/b/c") == "../../d");
	TEST_ASSERT(rod::path("/a/b/c").lexically_relative("/a/d") == "../b/c");

	TEST_ASSERT(rod::path("a/b/c").lexically_relative("a/b/c/x/y") == "../..");
	TEST_ASSERT(rod::path("a/b/c").lexically_relative("a/b/c") == ".");
	TEST_ASSERT(rod::path("a/b/c").lexically_relative("a") == "b/c");

	TEST_ASSERT(rod::path("a/b").lexically_relative("c/d") == "../../a/b");
	TEST_ASSERT(rod::path("a/b").lexically_proximate("/a/b") == "a/b");
	TEST_ASSERT(rod::path("a/b").lexically_relative("/a/b") == "");
}
