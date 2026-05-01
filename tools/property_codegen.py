#!/usr/bin/env python3
"""
Scan src/**/*.h for META_CLASS() and /// @property tags; emit src/Codegen/<Stem>.generated.hpp.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any

CACHE_NAME = ".codegen_cache.json"
CACHE_VERSION = 1

PROPERTY_TAG_RE = re.compile(r"^\s*///\s*@property\s*(?:\((.*)\))?\s*$")
META_CLASS_RE = re.compile(r"\bMETA_CLASS\s*\(\s*\)")
CLASS_HEAD_RE = re.compile(
    r"^\s*(?:template\s*<[^>{};]*>\s*)?(?:class|struct)\s+([A-Za-z_]\w*)\b"
)
NS_HEAD_RE = re.compile(r"^\s*namespace\s+([A-Za-z_]\w*)\s*(?:\{)?\s*$")
FIELD_RE = re.compile(
    r"^\s*(?:inline\s+|static\s+|mutable\s+)*"
    r"(float|double|bool|int|std::int32_t|std::int64_t|std::string|sf::Vector2f|sf::Vector3f|sf::Color)\s+"
    r"(\w+)\s*.*;\s*$"
)
KNOWN_TYPES = frozenset(
    {
        "float",
        "double",
        "bool",
        "int",
        "std::int32_t",
        "std::int64_t",
        "std::string",
        "sf::Vector2f",
        "sf::Vector3f",
        "sf::Color",
    }
)


@dataclass
class PropSpec:
    cpp_type: str
    member: str
    line: int
    col: int
    attrs: dict[str, Any] = field(default_factory=dict)


@dataclass
class ClassSpec:
    namespaces: tuple[str, ...]
    class_name: str
    props: list[PropSpec] = field(default_factory=list)

    def qualified(self) -> str:
        return "::".join((*self.namespaces, self.class_name))


def pascal_to_snake(name: str) -> str:
    out: list[str] = []
    for i, c in enumerate(name):
        if c.isupper() and i > 0:
            prev = name[i - 1]
            nxt = name[i + 1] if i + 1 < len(name) else ""
            if prev.islower() or (nxt and nxt.islower()):
                out.append("_")
        out.append(c.lower())
    return "".join(out)


def member_to_field_id(member: str) -> str:
    return pascal_to_snake(member.lstrip("_"))


def cpp_escape_string(s: str) -> str:
    return s.replace("\\", "\\\\").replace('"', '\\"')


def count_braces_outside_strings(line: str) -> tuple[int, int]:
    opens = closes = 0
    i = 0
    state: str | None = None
    while i < len(line):
        c = line[i]
        if state == '"':
            if c == "\\" and i + 1 < len(line):
                i += 2
                continue
            if c == '"':
                state = None
            i += 1
            continue
        if state == "'":
            if c == "\\" and i + 1 < len(line):
                i += 2
                continue
            if c == "'":
                state = None
            i += 1
            continue
        if c == '"':
            state = '"'
            i += 1
            continue
        if c == "'":
            state = "'"
            i += 1
            continue
        if c == "/" and i + 1 < len(line) and line[i + 1] == "/":
            break
        if c == "{":
            opens += 1
        elif c == "}":
            closes += 1
        i += 1
    return opens, closes


def strip_line_comment_keep_doc(line: str) -> str:
    if line.lstrip().startswith("///"):
        return line
    if "//" in line:
        return line.split("//", 1)[0]
    return line


class ParseError(Exception):
    def __init__(self, message: str, path: Path, line: int, col: int) -> None:
        super().__init__(f"{path}:{line}:{col}: {message}")
        self.path = path
        self.line = line
        self.col = col


def _skip_ws(s: str, i: int) -> int:
    while i < len(s) and s[i] in " \t\r\n":
        i += 1
    return i


def parse_attr_dict(s: str, path: Path, base_line: int, base_col: int) -> dict[str, Any]:
    s = s.strip()
    if not s:
        return {}
    out: dict[str, Any] = {}
    i = 0
    line = base_line
    col = base_col + 1

    def err(msg: str, at: int) -> None:
        raise ParseError(msg, path, line, col + at)

    while i < len(s):
        i = _skip_ws(s, i)
        if i >= len(s):
            break
        key_start = i
        while i < len(s) and (s[i].isalnum() or s[i] == "_"):
            i += 1
        if i == key_start:
            err("expected key name", i)
        key = s[key_start:i]
        i = _skip_ws(s, i)
        if i >= len(s) or s[i] != "=":
            err(f"expected '=' after key '{key}'", i - key_start)
        i += 1
        i = _skip_ws(s, i)
        if i >= len(s):
            err(f"missing value for '{key}'", 0)

        if s[i] == '"':
            i += 1
            buf: list[str] = []
            while i < len(s):
                if s[i] == "\\" and i + 1 < len(s):
                    buf.append(s[i + 1])
                    i += 2
                    continue
                if s[i] == '"':
                    break
                buf.append(s[i])
                i += 1
            if i >= len(s):
                err("unterminated string", i - key_start)
            val = "".join(buf)
            i += 1
        else:
            val_start = i
            while i < len(s) and s[i] not in ", \t\r\n":
                i += 1
            raw = s[val_start:i].strip()
            if raw in ("true", "false"):
                val = raw == "true"
            else:
                try:
                    val = float(raw.rstrip("fF")) if ("." in raw or "e" in raw.lower()) else int(raw)
                except ValueError:
                    val = raw
        out[key] = val
        i = _skip_ws(s, i)
        if i >= len(s):
            break
        if s[i] != ",":
            err(f"expected ',' or end after value for '{key}'", i - key_start)
        i += 1
    return out


def find_innermost_class(stack: list[dict[str, Any]]) -> dict[str, Any] | None:
    for b in reversed(stack):
        if b.get("kind") == "class":
            return b
    return None


def line_opens_class(line: str) -> str | None:
    line = strip_line_comment_keep_doc(line)
    stripped = line.strip()
    if stripped.endswith(";"):
        return None
    if re.search(r"\benum\s+class\b", line):
        return None
    m = CLASS_HEAD_RE.match(line)
    if not m:
        return None
    return m.group(1)


def line_opens_namespace(line: str) -> str | None:
    line = strip_line_comment_keep_doc(line)
    m = NS_HEAD_RE.match(line)
    if not m or "namespace" not in line:
        return None
    return m.group(1)


def classify_open_brace(
    line: str,
    brace_idx: int,
    pending_class: str | None,
    pending_ns: str | None,
) -> tuple[str, str | None]:
    """Return (kind, name) for block opened at brace_idx in line."""
    prefix = line[:brace_idx]
    if pending_ns is not None and "namespace" in prefix and pending_ns in prefix:
        return "namespace", pending_ns
    if pending_class is not None:
        return "class", pending_class
    pl = strip_line_comment_keep_doc(prefix)
    m_ns = NS_HEAD_RE.match(pl)
    if m_ns:
        return "namespace", m_ns.group(1)
    m_cl = CLASS_HEAD_RE.match(pl)
    if m_cl and not pl.strip().endswith(";"):
        return "class", m_cl.group(1)
    return "other", None


def parse_header(path: Path) -> tuple[list[ClassSpec], list[str]]:
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    log: list[str] = []

    ns_stack: list[str] = []
    block_stack: list[dict[str, Any]] = []
    pending_class: str | None = None
    pending_ns: str | None = None
    finished: list[ClassSpec] = []
    pending_prop: tuple[int, int, str] | None = None

    def close_class_block(b: dict[str, Any]) -> None:
        if not b.get("meta"):
            return
        finished.append(
            ClassSpec(
                namespaces=tuple(b["namespaces"]),
                class_name=b["name"],
                props=list(b.get("props", [])),
            )
        )

    for i, raw_line in enumerate(lines):
        line_no = i + 1
        line = strip_line_comment_keep_doc(raw_line)
        stripped = line.strip()

        if pending_prop is not None:
            pline, pcol, pargs = pending_prop
            if not stripped or stripped.startswith("///"):
                continue
            if re.match(r"^\s*(public|private|protected)\s*:\s*$", line):
                continue
            m_field = FIELD_RE.match(line)
            if not m_field:
                raise ParseError(
                    f"expected field with supported type after @property (tag at line {pline})",
                    path,
                    line_no,
                    1,
                )
            cpp_type, member = m_field.group(1), m_field.group(2)
            inner = find_innermost_class(block_stack)
            if inner is None or not inner.get("meta"):
                raise ParseError("@property must appear inside a class marked with META_CLASS()", path, pline, pcol)
            attrs = parse_attr_dict(pargs, path, pline, pcol)
            inner.setdefault("props", []).append(
                PropSpec(cpp_type=cpp_type, member=member, line=pline, col=pcol, attrs=attrs)
            )
            pending_prop = None
            continue

        if PROPERTY_TAG_RE.match(raw_line):
            if pending_prop is not None:
                pl, pc, _ = pending_prop
                raise ParseError(
                    "unfinished @property (another @property started before the field line)",
                    path,
                    line_no,
                    1,
                )
            args = PROPERTY_TAG_RE.match(raw_line).group(1) or ""
            try:
                col = raw_line.index("@property") + 1
            except ValueError:
                col = 1
            pending_prop = (line_no, col, args)
            continue

        if META_CLASS_RE.search(line) and not re.search(r"^\s*#\s*define\s+META_CLASS\b", line):
            inner = find_innermost_class(block_stack)
            if inner is None:
                raise ParseError("META_CLASS() must appear inside a class definition", path, line_no, 1)
            inner["meta"] = True
            continue

        p_ns = line_opens_namespace(raw_line)
        if p_ns is not None:
            opens, _ = count_braces_outside_strings(raw_line)
            if "{" in raw_line and opens > 0:
                ns_stack.append(p_ns)
                block_stack.append({"kind": "namespace", "name": p_ns})
                pending_ns = None
            else:
                pending_ns = p_ns
            continue

        if pending_ns and stripped.startswith("{"):
            ns_stack.append(pending_ns)
            block_stack.append({"kind": "namespace", "name": pending_ns})
            pending_ns = None
            continue

        p_cl = line_opens_class(raw_line)
        if p_cl is not None:
            opens, _ = count_braces_outside_strings(raw_line)
            if "{" in raw_line and opens > 0:
                block_stack.append(
                    {
                        "kind": "class",
                        "name": p_cl,
                        "meta": False,
                        "props": [],
                        "namespaces": tuple(ns_stack),
                    }
                )
                pending_class = None
            else:
                pending_class = p_cl
            continue

        if pending_class and stripped.startswith("{"):
            block_stack.append(
                {
                    "kind": "class",
                    "name": pending_class,
                    "meta": False,
                    "props": [],
                    "namespaces": tuple(ns_stack),
                }
            )
            pending_class = None
            continue

        scan = strip_line_comment_keep_doc(raw_line)
        j = 0
        while j < len(scan):
            if scan[j] == "{":
                kind, name = classify_open_brace(scan, j, pending_class, pending_ns)
                if kind == "namespace" and name:
                    if pending_ns == name:
                        pending_ns = None
                    if not ns_stack or ns_stack[-1] != name:
                        ns_stack.append(name)
                    block_stack.append({"kind": "namespace", "name": name})
                elif kind == "class" and name:
                    if pending_class == name:
                        pending_class = None
                    block_stack.append(
                        {
                            "kind": "class",
                            "name": name,
                            "meta": False,
                            "props": [],
                            "namespaces": tuple(ns_stack),
                        }
                    )
                else:
                    block_stack.append({"kind": "other"})
                j += 1
                continue
            if scan[j] == "}":
                if not block_stack:
                    j += 1
                    continue
                popped = block_stack.pop()
                if popped["kind"] == "class":
                    close_class_block(popped)
                elif popped["kind"] == "namespace":
                    if ns_stack:
                        ns_stack.pop()
                j += 1
                continue
            j += 1

    if pending_prop:
        pline, pcol, _ = pending_prop
        raise ParseError("unfinished @property (no field declaration before EOF)", path, pline, pcol)

    for spec in finished:
        log.append(f"  class {spec.qualified()} : {len(spec.props)} properties")

    return finished, log


def default_label(member: str, attrs: dict[str, Any]) -> str:
    if "name" in attrs and isinstance(attrs["name"], str):
        return attrs["name"]
    if "label" in attrs and isinstance(attrs["label"], str):
        return attrs["label"]
    fid = member_to_field_id(member)
    return fid.replace("_", " ").title()


def format_meta_inline(p: PropSpec) -> str:
    a = p.attrs
    parts: list[str] = ["Engine::PropertyMeta _m;"]
    if a.get("readonly") is True:
        parts.append("_m.readOnly = true;")
    if isinstance(a.get("tooltip"), str):
        parts.append(f'_m.tooltip = "{cpp_escape_string(a["tooltip"])}";')
    for key, mk in (
        ("minValue", "numericMin"),
        ("maxValue", "numericMax"),
        ("step", "numericStep"),
        ("dragSpeed", "dragSpeed"),
    ):
        if key not in a:
            continue
        v = a[key]
        if isinstance(v, (int, float)):
            parts.append(f"_m.{mk} = static_cast<double>({float(v)});")
    inner = " ".join(parts)
    return f"[&]() -> Engine::PropertyMeta {{ {inner} return _m; }}()"


def generate_file_content(path: Path, classes: list[ClassSpec]) -> str:
    out: list[str] = [
        "// Generated by tools/property_codegen.py — do not edit.",
        "#pragma once",
        "",
        '#include "Engine/Core/PropertyTree.h"',
        "",
        "#include <functional>",
        "",
    ]
    needs_sfml = any(p.cpp_type.startswith("sf::") for c in classes for p in c.props)
    if needs_sfml:
        out.append('#include <SFML/Graphics/Color.hpp>')
        out.append('#include <SFML/System/Vector2.hpp>')
        out.append('#include <SFML/System/Vector3.hpp>')
        out.append("")
    out.append("#include <cstdint>")
    out.append("#include <string>")
    out.append("")

    for c in classes:
        q = c.qualified()
        root_id = pascal_to_snake(c.class_name)
        root_label = cpp_escape_string(c.class_name)
        out.append(f"void {q}::BuildPropertyTree(Engine::PropertyBuilder& b) {{")
        out.append(f'\tb.pushObject("{root_id}", "{root_label}");')
        for p in c.props:
            if p.cpp_type not in KNOWN_TYPES:
                raise ParseError(f"unsupported type `{p.cpp_type}`", path, p.line, p.col)
            a = p.attrs
            has_meta = a.get("readonly") is True or any(
                k in a for k in ("tooltip", "minValue", "maxValue", "step", "dragSpeed")
            )
            meta_arg = "Engine::PropertyMeta{}" if not has_meta else format_meta_inline(p)
            get_lambda = "[this] { return " + p.member + "; }"
            readonly = a.get("readonly") is True
            setters_ro = {
                "float": "[this](float) {}",
                "double": "[this](double) {}",
                "bool": "[this](bool) {}",
                "int": "[this](std::int32_t) {}",
                "std::int32_t": "[this](std::int32_t) {}",
                "std::int64_t": "[this](std::int64_t) {}",
                "std::string": "[this](std::string) {}",
                "sf::Vector2f": "[this](sf::Vector2f) {}",
                "sf::Vector3f": "[this](sf::Vector3f) {}",
                "sf::Color": "[this](sf::Color) {}",
            }
            setters_rw = {
                "float": f"[this](float v) {{ {p.member} = v; }}",
                "double": f"[this](double v) {{ {p.member} = v; }}",
                "bool": f"[this](bool v) {{ {p.member} = v; }}",
                "int": f"[this](std::int32_t v) {{ {p.member} = static_cast<int>(v); }}",
                "std::int32_t": f"[this](std::int32_t v) {{ {p.member} = v; }}",
                "std::int64_t": f"[this](std::int64_t v) {{ {p.member} = v; }}",
                "std::string": f"[this](std::string v) {{ {p.member} = std::move(v); }}",
                "sf::Vector2f": f"[this](sf::Vector2f v) {{ {p.member} = v; }}",
                "sf::Vector3f": f"[this](sf::Vector3f v) {{ {p.member} = v; }}",
                "sf::Color": f"[this](sf::Color c) {{ {p.member} = c; }}",
            }
            setter_method = a.get("setter")
            if (
                isinstance(setter_method, str)
                and setter_method
                and not readonly
                and p.cpp_type == "bool"
            ):
                set_lambda = f"[this](bool v) {{ this->{setter_method}(v); }}"
            else:
                set_lambda = setters_ro[p.cpp_type] if readonly else setters_rw[p.cpp_type]
            fid = member_to_field_id(p.member)
            label_esc = cpp_escape_string(default_label(p.member, a))
            if p.cpp_type == "float":
                out.append(f'\tb.addFloat("{fid}", "{label_esc}", {get_lambda}, {set_lambda}, {meta_arg});')
            elif p.cpp_type == "double":
                out.append(f'\tb.addDouble("{fid}", "{label_esc}", {get_lambda}, {set_lambda}, {meta_arg});')
            elif p.cpp_type == "bool":
                out.append(f'\tb.addBool("{fid}", "{label_esc}", {get_lambda}, {set_lambda}, {meta_arg});')
            elif p.cpp_type in ("int", "std::int32_t"):
                out.append(f'\tb.addInt32("{fid}", "{label_esc}", {get_lambda}, {set_lambda}, {meta_arg});')
            elif p.cpp_type == "std::int64_t":
                out.append(f'\tb.addInt64("{fid}", "{label_esc}", {get_lambda}, {set_lambda}, {meta_arg});')
            elif p.cpp_type == "std::string":
                out.append(f'\tb.addString("{fid}", "{label_esc}", {get_lambda}, {set_lambda}, {meta_arg});')
            elif p.cpp_type == "sf::Vector2f":
                out.append(f'\tb.addVec2f("{fid}", "{label_esc}", {get_lambda}, {set_lambda}, {meta_arg});')
            elif p.cpp_type == "sf::Vector3f":
                out.append(f'\tb.addVec3f("{fid}", "{label_esc}", {get_lambda}, {set_lambda}, {meta_arg});')
            elif p.cpp_type == "sf::Color":
                out.append(f'\tb.addColor("{fid}", "{label_esc}", {get_lambda}, {set_lambda}, {meta_arg});')
        out.append("\tb.pop();")
        out.append("}")
        out.append("")

    return "\n".join(out).rstrip() + "\n"


def load_cache(cache_path: Path) -> dict[str, Any]:
    if not cache_path.is_file():
        return {"version": CACHE_VERSION, "files": {}}
    try:
        data = json.loads(cache_path.read_text(encoding="utf-8"))
    except (OSError, json.JSONDecodeError):
        return {"version": CACHE_VERSION, "files": {}}
    if data.get("version") != CACHE_VERSION:
        return {"version": CACHE_VERSION, "files": {}}
    return data


def save_cache(cache_path: Path, data: dict[str, Any]) -> None:
    cache_path.parent.mkdir(parents=True, exist_ok=True)
    cache_path.write_text(json.dumps(data, indent=2), encoding="utf-8")


def file_signature(p: Path) -> dict[str, Any]:
    st = p.stat()
    return {"mtime_ns": st.st_mtime_ns, "size": st.st_size}


def main() -> int:
    ap = argparse.ArgumentParser(description="Property tree codegen for TestEngine.")
    ap.add_argument("--root", type=Path, default=Path.cwd(), help="Repository root (default: cwd)")
    ap.add_argument("--force", action="store_true", help="Ignore cache and regenerate all headers")
    ap.add_argument("--verbose", action="store_true", help="Log every scanned header (default: summary only)")
    args = ap.parse_args()
    root: Path = args.root.resolve()
    src = root / "src"
    codegen_dir = src / "Codegen"
    cache_path = codegen_dir / CACHE_NAME

    if not src.is_dir():
        print(f"[codegen] ERROR: missing src directory: {src}", file=sys.stderr)
        return 1

    codegen_dir.mkdir(parents=True, exist_ok=True)

    cache = {"version": CACHE_VERSION, "files": {}} if args.force else load_cache(cache_path)
    cache_files: dict[str, Any] = dict(cache.get("files", {}))

    headers = sorted(
        p
        for p in src.rglob("*.h")
        if "Codegen" not in p.parts and p.is_file()
    )
    print(f"[codegen] root={root}")
    print(f"[codegen] scanning {len(headers)} headers under src/")
    touched = 0
    skipped_cache = 0

    for h in headers:
        rel = h.relative_to(root).as_posix()
        out_path = codegen_dir / f"{h.stem}.generated.hpp"
        sig = file_signature(h)
        cached = cache_files.get(rel)
        if (
            not args.force
            and cached
            and cached.get("mtime_ns") == sig["mtime_ns"]
            and cached.get("size") == sig["size"]
            and out_path.is_file()
        ):
            skipped_cache += 1
            if args.verbose:
                print(f"[codegen] skip (cached) {rel}")
            cache_files[rel] = sig
            continue

        try:
            classes, clog = parse_header(h)
        except ParseError as e:
            print(f"[codegen] ERROR: {e}", file=sys.stderr)
            return 1

        meta_only = [c for c in classes]
        if not meta_only:
            if args.verbose:
                print(f"[codegen] no META_CLASS properties in {rel}")
            cache_files[rel] = sig
            if out_path.is_file():
                try:
                    out_path.unlink()
                except OSError:
                    pass
            continue

        touched += 1
        for line in clog:
            print(f"[codegen] {rel}: {line}")

        try:
            content = generate_file_content(h, meta_only)
        except ParseError as e:
            print(f"[codegen] ERROR: {e}", file=sys.stderr)
            return 1

        old = out_path.read_text(encoding="utf-8") if out_path.is_file() else None
        if old != content:
            out_path.write_text(content, encoding="utf-8")
            print(f"[codegen] wrote {out_path.relative_to(root)}")
        else:
            print(f"[codegen] unchanged {out_path.relative_to(root)}")
        cache_files[rel] = sig

    cache_files = {k: cache_files[k] for k in sorted(cache_files)}
    save_cache(cache_path, {"version": CACHE_VERSION, "files": cache_files})
    print(
        f"[codegen] done. headers_with_meta={touched}, skipped_cached={skipped_cache}, "
        f"cache={cache_path.relative_to(root)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
