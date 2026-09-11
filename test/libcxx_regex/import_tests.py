#!/usr/bin/env python3
"""Import the narrow-character libc++ regex tests from a pinned LLVM checkout.

Run once when updating the vendored port, not as part of the build. The test
sources, manifest, support headers, and license are reproducible from this script.
"""

import argparse
import json
import re
import subprocess
from pathlib import Path

REVISION = "242ccbf6b6d9d090762a87138c854f2d93b6ab60"
ROOT = Path(__file__).resolve().parent


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("llvm_checkout", type=Path)
    args = parser.parse_args()
    checkout = args.llvm_checkout.resolve()
    revision = subprocess.check_output(
        ["git", "-C", str(checkout), "rev-parse", "HEAD"], text=True
    ).strip()
    if revision != REVISION:
        parser.error(f"expected LLVM revision {REVISION}, got {revision}")
    source = checkout / "libcxx/test/std/re"
    manifest = {"revision": REVISION, "tests": []}
    support = set()
    for path in sorted(source.rglob("*.cpp")):
        relative = path.relative_to(source).as_posix()
        text = path.read_text()
        reason = None
        value_construction = (
            relative.startswith("re.results/re.results.const/")
            and path.name != "allocator.pass.cpp"
        )
        locale_test = (
            relative.startswith("re.traits/")
            or relative == "re.regex/re.regex.locale/imbue.pass.cpp"
        )
        if "nothing_to_do" in relative or "tested_elsewhere" in relative:
            reason = "Upstream placeholder; no independent assertions."
        elif relative.startswith("re.syn/w"):
            reason = "Wide-character API is not implemented."
        elif "deduct" in relative:
            reason = "Pattern capacity must be supplied explicitly; STL basic_regex CTAD is inapplicable."
        elif "get_allocator" in text and not value_construction:
            reason = "Allocator construction/access has no fixed-capacity equivalent."
        elif "std::locale(" in text and not locale_test:
            reason = (
                "Named std::locale facets are not supported by the value-table locale."
            )
        elif relative.endswith("empty.verify.cpp"):
            reason = "Implementation-specific nodiscard warning diagnostic, not a behavioral test."
        if reason:
            manifest["tests"].append(
                {"source": relative, "status": "not_applicable", "reason": reason}
            )
            continue
        text = text.replace("#include <regex>", '#include "adapter.h"')
        names = (
            "regex_traits",
            "basic_regex",
            "regex",
            "sub_match",
            "match_results",
            "regex_iterator",
            "regex_token_iterator",
            "csub_match",
            "ssub_match",
            "cmatch",
            "smatch",
            "cregex_iterator",
            "sregex_iterator",
            "cregex_token_iterator",
            "sregex_token_iterator",
            "regex_match",
            "regex_search",
            "regex_replace",
        )
        text = re.sub(
            r"std::(" + "|".join(names) + r")\b", r"libcxx_fixed_regex::\1", text
        )
        notes = [
            "Use fixed-capacity types/algorithms via adapter.h; disable guarded wchar_t cases."
        ]
        if value_construction:
            text = text.replace('#include "test_allocator.h"\n', "")
            text = re.sub(
                r"^    typedef std::allocator<.*Alloc;\n", "", text, flags=re.M
            )
            text = text.replace("class CharT, class Allocator", "class CharT")
            text = text.replace("test(const Allocator& a)", "test()")
            text = text.replace(
                "match_results<const CharT*, Allocator>", "match_results<const CharT*>"
            )
            text = text.replace("SM m0(a);", "SM m0;")
            text = re.sub(
                r"    if \(std::allocator_traits<Allocator>.*?\n    else\n        assert\([^\n]*\);\n",
                "",
                text,
                flags=re.S,
            )
            text = re.sub(
                r"^.*(?:assert.*get_allocator|static_assert.*allocator_type).*$",
                "",
                text,
                flags=re.M,
            )
            if path.name != "default.pass.cpp":
                text = (
                    text[: text.index("int main(")]
                    + "int main(int, char**)\n{\n    test<char>();\n    return 0;\n}\n"
                )
            notes.append(
                "Retain default/copy/move state and noexcept assertions; remove allocator construction, access, propagation, and allocator-count checks."
            )
        if locale_test and "std::locale(" in text:
            text = re.sub(r'^#include "platform_support.h".*\n', "", text, flags=re.M)
            if path.name in ("default.pass.cpp", "getloc.pass.cpp"):
                text = (
                    text[: text.index("int main(")]
                    + """int main(int, char**)
{
    libcxx_fixed_regex::regex_traits<char> t;
    assert(t.getloc() == fixed_containers::FixedRegexLocale{});
    t.imbue(libcxx_fixed_regex::test_locale());
    assert(t.getloc() == libcxx_fixed_regex::test_locale());
    return 0;
}
"""
                )
            else:
                text = re.sub(
                    r"std::locale\(LOCALE_[A-Za-z0-9_]+\)",
                    "libcxx_fixed_regex::test_locale()",
                    text,
                )
                text = text.replace(
                    'std::locale("C")', "fixed_containers::FixedRegexLocale{}"
                )
                text = text.replace("std::locale loc", "auto loc")
                text = text.replace(
                    '.name() == "C"', " == fixed_containers::FixedRegexLocale{}"
                )
                text = re.sub(
                    r"\.name\(\) == LOCALE_[A-Za-z0-9_]+",
                    " == libcxx_fixed_regex::test_locale()",
                    text,
                )
            notes.append(
                "Use explicit fixed value-table locale fixture instead of named OS facets/global locale; retain narrow transform/imbue/case assertions. No claim of named-locale equivalence."
            )
        text, changes = re.subn(
            r"^\s*test<wchar_t>\(\);",
            "\n    // wchar_t is not supported.",
            text,
            flags=re.M,
        )
        if changes:
            notes.append("Omit unguarded wchar_t instantiation.")
        text, changes = re.subn(
            r"std::string (\w+) = (libcxx_fixed_regex::regex_replace)",
            r"auto \1 = \2",
            text,
        )
        if changes:
            notes.append(
                "Owning replacement results use auto (FixedString), not std::string."
            )
        if relative in {
            "re.results/types.pass.cpp",
            "re.submatch/types.pass.cpp",
            "re.traits/types.pass.cpp",
        }:
            text = re.sub(r"^.*static_assert.*allocator_type.*\n", "", text, flags=re.M)
            text = text.replace(
                "std::basic_string<CharT>", "fixed_containers::FixedString<256>"
            )
            text = text.replace(
                "string_type, std::string",
                "string_type, fixed_containers::FixedString<256>",
            )
            text = text.replace("std::locale", "fixed_containers::FixedRegexLocale")
            text = text.replace(
                "    test<wchar_t>();", "    // wchar_t is not supported."
            )
            notes.append(
                "Check fixed string/locale aliases; omit allocator alias and unguarded wchar_t instantiation."
            )
        if relative == "re.regex/types.pass.cpp":
            text = text.replace("std::locale", "fixed_containers::FixedRegexLocale")
            text = text.replace(
                "std::basic_string<char>", "fixed_containers::FixedString<256>"
            )
            text = re.sub(
                r"    static_assert\(\(std::is_same<libcxx_fixed_regex::basic_regex<wchar_t>.*?;\n",
                "",
                text,
                flags=re.S,
            )
            notes.append(
                "Check fixed string/value-table locale aliases; omit unguarded wchar_t assertions."
            )
        if relative in {
            "re.results/re.results.form/form3.pass.cpp",
            "re.results/re.results.form/form4.pass.cpp",
        }:
            text = re.sub(
                r"(?:nstr|std::string) out = m.format", "auto out = m.format", text
            )
            notes.append(
                "Formatted owning output is FixedString; preserve custom-allocator input format strings."
            )
        if relative == "re.results/re.results.acc/str.pass.cpp":
            text = text.replace(
                "std::string(m[", "libcxx_fixed_regex::cmatch::string_type(m["
            )
            notes.append(
                "Compare against the fixed submatch's owning string conversion."
            )
        if relative == "re.results/range_concept_conformance.compile.pass.cpp":
            text = text.replace(
                "static_assert(std::ranges::viewable_range<libcxx_fixed_regex::cmatch>);",
                "// P2415R2 permits movable, non-borrowed rvalue ranges (libstdc++ 12 and newer).\n"
                "#if __cpp_lib_ranges >= 202110L\n"
                "static_assert(std::ranges::viewable_range<libcxx_fixed_regex::cmatch>);\n"
                "#else\n"
                "static_assert(!std::ranges::viewable_range<libcxx_fixed_regex::cmatch>);\n"
                "#endif\n"
                "static_assert(std::ranges::viewable_range<libcxx_fixed_regex::cmatch&>);\n"
                "static_assert(std::ranges::viewable_range<const libcxx_fixed_regex::cmatch&>);",
            )
            notes.append(
                "Select rvalue viewable_range expectations using the P2415R2 feature-test macro; check lvalue ranges on all supported libraries."
            )
        if relative == "re.grammar/excessive_brace_count.pass.cpp":
            text = text.replace(
                "100000000000000000", "1000000000000000000000000000000000000000"
            )
            notes.append(
                "Overflow size_t rather than libc++'s smaller internal repeat counter."
            )
        if relative == "re.regex/re.regex.construct/bad_escape.pass.cpp":
            for escape in ("a", "e"):
                text = text.replace(
                    'assert(error_escape_thrown("[\\\\' + escape + ']"));',
                    'assert(!error_escape_thrown("[\\\\' + escape + ']"));',
                )
                text = text.replace(
                    'assert(error_escape_thrown("\\\\' + escape + '"));',
                    'assert(!error_escape_thrown("\\\\' + escape + '"));',
                )
            notes.append(
                "IdentityEscape permits a/e in current [re.grammar]; keep invalid c escapes rejected (https://eel.is/c++draft/re.grammar)."
            )
        if "[=M=]" in text:
            text = re.sub(
                r"assert\(!libcxx_fixed_regex::regex_(?:match|search)\([^;]*\[=M=\][^;]*;\s*assert\(m.size\(\) == 0\);",
                lambda m: m.group()
                .replace("assert(!", "assert(", 1)
                .replace("m.size() == 0", "m.size() == 1"),
                text,
            )
            notes.append(
                "FixedRegexLocale's default primary table folds ASCII case, so [=M=] includes m (unlike libc++'s classic primary key)."
            )
        if relative == "re.alg/re.alg.search/ecma.pass.cpp":
            start = text.index('const char s[] = "bc";')
            end = text.index("\n    }", start)
            block = text[start:end].replace("m.position(1) == 0", "m.position(1) == 2")
            block += "\n        assert(!m[1].matched); // ECMA-262 RepeatMatcher rejects optional empty iterations."
            text = text[:start] + block + text[end:]
            notes.append(
                "For (a*)* on bc, capture 1 is unmatched: ECMA-262 15.10.2.5, step 2.1. Position is end of input, not zero."
            )
        if relative == "re.const/re.matchflag/match_prev_avail.pass.cpp":
            text = re.sub(
                r"assert\(libcxx_fixed_regex::regex_(?:match|search)\([^;]*?;",
                lambda m: m.group().replace("assert(", "assert(!", 1)
                if 'regex("^' in m.group() and "match_prev_avail" in m.group()
                else m.group(),
                text,
            )
            text = text.replace(
                "// Assert that match_prev_avail disables match_not_bol and this matches",
                "// match_prev_avail supplies input context; ^ is not the start of the original input.",
            )
            notes.append(
                "Retain libstdc++/MSVC anchor semantics for match_prev_avail; libc++ differs. See LWG 3605 and LLVM issue 74838. Word-boundary assertions unchanged."
            )
        if relative == "re.traits/lookup_classname.pass.cpp":
            text = text.replace(
                "typename libcxx_fixed_regex::regex_traits<char_type>::char_class_type expected",
                "std::ctype_base::mask expected",
            )
            text = text.replace(
                "assert(result == expected);",
                """// char_class_type encodings are implementation-defined: compare membership.
    const auto& facet = std::use_facet<std::ctype<char>>(std::locale::classic());
    for (int i = 0; i < 256; ++i)
        assert(t.isctype(static_cast<char>(i), result) == facet.is(expected, static_cast<char>(i)));""",
            )
            start = text.index("    assert((result & expected) == expected);")
            end = text.index("\n}", start)
            text = (
                text[:start]
                + """    const auto& facet = std::use_facet<std::ctype<char>>(std::locale::classic());
    for (int i = 0; i < 256; ++i)
        assert(t.isctype(static_cast<char>(i), result) ==
               (i == '_' || facet.is(expected, static_cast<char>(i))));"""
                + text[end:]
            )
            notes.append(
                "Compare character-class membership, not libc++'s implementation-defined mask representation."
            )
        if relative.endswith(".compile.pass.cpp"):
            text += "\nint main() {} // The assertions above are compile-time tests.\n"
        text = text.replace(
            '#include "adapter.h"',
            "// Ported from LLVM "
            + REVISION
            + "/"
            + relative
            + '\n// See test/libcxx_regex/manifest.json for all adaptations to this test.\n#include "adapter.h"',
            1,
        )
        output = ROOT / "tests" / relative
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(text)
        support.update(re.findall(r'^#include "(?!adapter.h)([^"]+)"', text, re.M))
        kind = (
            "compile_fail"
            if ".compile.fail." in relative or ".verify." in relative
            else "pass"
        )
        manifest["tests"].append(
            {"source": relative, "status": kind, "adaptations": notes}
        )
    pending = list(support)
    while pending:
        name = pending.pop()
        text = (checkout / "libcxx/test/support" / name).read_text()
        output = ROOT / "support" / name
        output.parent.mkdir(parents=True, exist_ok=True)
        output.write_text(text)
        for dependency in re.findall(r'^#\s*include "([^"]+)"', text, re.M):
            if dependency not in support:
                support.add(dependency)
                pending.append(dependency)
    (ROOT / "LICENSE.TXT").write_text((checkout / "libcxx/LICENSE.TXT").read_text())
    (ROOT / "manifest.json").write_text(json.dumps(manifest, indent=2) + "\n")
    print(
        f"Inventoried {len(manifest['tests'])} tests; copied {len(support)} support headers."
    )


if __name__ == "__main__":
    main()
