#!/usr/bin/env python3
# SPDX-License-Identifier: MIT
"""Fail if gcovr's detailed HTML report has broken local asset/page links."""
from html.parser import HTMLParser
from pathlib import Path
import sys
from urllib.parse import unquote, urlsplit


class References(HTMLParser):
    def __init__(self):
        super().__init__()
        self.paths = []

    def handle_starttag(self, tag, attrs):
        attrs = dict(attrs)
        for key in ("href", "src"):
            value = attrs.get(key)
            if value:
                self.paths.append(value)


def main(folder):
    root = Path(folder).resolve()
    pages = sorted(root.rglob("*.html"))
    if not pages:
        raise ValueError(f"No HTML coverage pages found under {root}")
    missing = []
    for page in pages:
        parser = References()
        parser.feed(page.read_text(encoding="utf-8"))
        for value in parser.paths:
            url = urlsplit(value)
            if url.scheme or url.netloc or not url.path:
                continue
            target = (page.parent / unquote(url.path)).resolve()
            if root not in target.parents and target != root:
                missing.append(f"{page}: unsafe local link {value}")
            elif not target.is_file():
                missing.append(f"{page}: missing local link {value}")
    if missing:
        raise ValueError("Coverage HTML contains broken local links:\n" + "\n".join(missing))
    print(f"Coverage HTML links verified ({len(pages)} pages).")


if __name__ == "__main__":
    try:
        main(sys.argv[1] if len(sys.argv) > 1 else "coverage")
    except (OSError, ValueError, IndexError) as error:
        raise SystemExit(str(error))
