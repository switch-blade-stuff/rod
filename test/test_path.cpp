/*
 * Created by switch_blade on 2023-08-04.
 */

#include <rod/path.hpp>

#include "common.hpp"

int main()
{
	rod::fs::path_view view("//host/dir/name.ext");
#ifdef ROD_WIN32
	TEST_ASSERT(view.root_path() == rod::fs::path_view("//host/"));
	TEST_ASSERT(view.root_name() == rod::fs::path_view("//host"));
#else
	TEST_ASSERT(view.root_path() == rod::fs::path_view("/"));
	TEST_ASSERT(view.root_name() == rod::fs::path_view(""));
#endif
	TEST_ASSERT(view.root_directory() == rod::fs::path_view("/"));
	TEST_ASSERT(view.relative_path() == rod::fs::path_view("dir/name.ext"));
	TEST_ASSERT(view.parent_path() == rod::fs::path_view("//host/dir"));
	TEST_ASSERT(view.filename() == rod::fs::path_view("name.ext"));
	TEST_ASSERT(view.extension() == rod::fs::path_view(".ext"));
	TEST_ASSERT(view.stem() == rod::fs::path_view("name"));

	TEST_ASSERT(view.remove_extension() == rod::fs::path_view("//host/dir/name"));
	TEST_ASSERT(view.remove_filename() == rod::fs::path_view("//host/dir/"));

	const auto rendered_wchar = view.render_null_terminated<wchar_t>();
	const auto rendered_char8 = view.render_null_terminated<char8_t>();
	const auto rendered_char16 = view.render_null_terminated<char16_t>();
	const auto rendered_char32 = view.render_null_terminated<char32_t>();

	TEST_ASSERT(std::wstring_view(L"//host/dir/name.ext") == rendered_wchar.c_str());
	TEST_ASSERT(std::u8string_view(u8"//host/dir/name.ext") == rendered_char8.c_str());
	TEST_ASSERT(std::u16string_view(u"//host/dir/name.ext") == rendered_char16.c_str());
	TEST_ASSERT(std::u32string_view(U"//host/dir/name.ext") == rendered_char32.c_str());
	TEST_ASSERT(view.compare(rod::fs::path_view(L"//host/dir//name.ext")) == 0);

	auto path = rod::fs::path("//host/dir/name.ext", std::locale());

	/* Win32 supports UNC syntax */
#ifdef ROD_WIN32
	TEST_ASSERT(path.root_path() == "//host/");
	TEST_ASSERT(path.root_name() == "//host");
#else
	TEST_ASSERT(path.root_path() == "/");
	TEST_ASSERT(path.root_name() == "");
#endif
	TEST_ASSERT(path.root_directory() == rod::fs::path("/"));
	TEST_ASSERT(path.relative_path() == rod::fs::path("dir/name.ext"));
	TEST_ASSERT(path.parent_path() == rod::fs::path("//host/dir"));
	TEST_ASSERT(path.filename() == rod::fs::path("name.ext"));
	TEST_ASSERT(path.extension() == rod::fs::path(".ext"));
	TEST_ASSERT(path.stem() == rod::fs::path("name"));
	TEST_ASSERT(rod::fs::path(view) == path);

	path = (path / "../subdir/./name.ext1").lexically_normal();
	TEST_ASSERT(path == rod::fs::path("//host/dir/subdir/name.ext1").lexically_normal());

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

	TEST_ASSERT(rod::fs::path("/a/d").lexically_relative("/a/b/c") == rod::fs::path("../../d"));
	TEST_ASSERT(rod::fs::path("/a/b/c").lexically_relative("/a/d") == rod::fs::path("../b/c"));

	TEST_ASSERT(rod::fs::path("a/b/c").lexically_relative("a/b/c/x/y") == rod::fs::path("../.."));
	TEST_ASSERT(rod::fs::path("a/b/c").lexically_relative("a/b/c") == rod::fs::path("."));
	TEST_ASSERT(rod::fs::path("a/b/c").lexically_relative("a") == rod::fs::path("b/c"));

	TEST_ASSERT(rod::fs::path("a/b").lexically_relative("c/d") == rod::fs::path("../../a/b"));
	TEST_ASSERT(rod::fs::path("a/b").lexically_proximate("/a/b") == rod::fs::path("a/b"));
	TEST_ASSERT(rod::fs::path("a/b").lexically_relative("/a/b") == rod::fs::path(""));
}
