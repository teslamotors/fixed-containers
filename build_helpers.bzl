def includes_config():
    return select({
        "//conditions:default": ["include"],
        ":strip_include_prefix_instead_of_includes": [],
    })

def strip_include_prefix_config():
    return select({
        "//conditions:default": None,
        ":strip_include_prefix_instead_of_includes": "include",
    })
