#!/usr/bin/env python3
"""
Scan src/**/*.h for META_CLASS(), /// @property, /// @getter, and /// @setter tags; emit src/Codegen/<Stem>.generated.hpp.
"""

from __future__ import annotations

import argparse
import json
import re
import sys
from dataclasses import dataclass, field
from pathlib import Path
from typing import Any, Literal

CACHE_NAME = ".codegen_cache.json"
CACHE_VERSION = 1

PROPERTY_TAG_RE = re.compile(r"^\s*///\s*@property\s*(?:\((.*)\))?\s*$")
GETTER_TAG_RE = re.compile(r"^\s*///\s*@getter\s*(?:\((.*)\))?\s*$")
SETTER_TAG_RE = re.compile(r"^\s*///\s*@setter\s*(?:\((.*)\))?\s*$")
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
_VEC_ELT = r"(float|double|bool|int|std::int32_t|std::int64_t|std::string|sf::Vector2f|sf::Vector3f|sf::Color)"
VECTOR_FIELD_RE = re.compile(
    r"^\s*(?:inline\s+|static\s+|mutable\s+)*std::vector<\s*"
    + _VEC_ELT
    + r"\s*>\s+(\w+)\s*;\s*$"
)
MAP_FIELD_RE = re.compile(
    r"^\s*(?:inline\s+|static\s+|mutable\s+)*(std::map|std::unordered_map)\s*<\s*"
    + _VEC_ELT
    + r"\s*,\s*"
    + _VEC_ELT
    + r"\s*>\s+(\w+)\s*(?:\{[^}]*\}|=\s*[^;]+)?\s*;\s*$"
)
SET_FIELD_RE = re.compile(
    r"^\s*(?:inline\s+|static\s+|mutable\s+)*(std::set|std::unordered_set)\s*<\s*"
    + _VEC_ELT
    + r"\s*>\s+(\w+)\s*(?:\{[^}]*\}|=\s*[^;]+)?\s*;\s*$"
)
BITSET_FIELD_RE = re.compile(
    r"^\s*(?:inline\s+|static\s+|mutable\s+)*std::bitset\s*<\s*[^>]+\s*>\s+(\w+)\s*(?:\{[^}]*\}|=\s*[^;]+)?\s*;\s*$"
)
# `using MyBits = std::bitset<...>;` inside the same class — fields may use `MyBits name;`
USING_BITSET_ALIAS_RE = re.compile(
    r"^\s*using\s+(\w+)\s*=\s*std::bitset\s*<\s*[^>]+\s*>\s*;\s*$"
)
BITSET_TYPEDEF_FIELD_RE = re.compile(
    r"^\s*(?:inline\s+|static\s+|mutable\s+)*(\w+)\s+(\w+)\s*(?:\{[^}]*\}|=\s*[^;]+)?\s*;\s*$"
)
# One-line non-static method declaration; parameter lists must not contain ')' inside nested parens (v1).
# Return type: known scalar/vector name, optionally "const" before and/or "&" after (e.g. const sf::Vector2f&).
GETTER_METHOD_RE = re.compile(
    r"^\s*(?:\[\[[^\]]*\]\]\s+)*(?!static\b)(?:virtual\s+)?"
    r"(?:const\s+)?"
    r"(float|double|bool|int|std::int32_t|std::int64_t|std::string|sf::Vector2f|sf::Vector3f|sf::Color)"
    r"(?:\s*&)?\s+"
    r"(\w+)\s*\([^)]*\)\s*"
    r"(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\s*;\s*$"
)
# std::vector<Elt> Method(...) const; or const std::vector<Elt>& Method(...) const;
GETTER_VECTOR_VAL_RE = re.compile(
    r"^\s*(?:\[\[[^\]]*\]\]\s+)*(?!static\b)(?:virtual\s+)?"
    r"std::vector<\s*" + _VEC_ELT + r"\s*>\s+"
    r"(\w+)\s*\([^)]*\)\s*(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\s*;\s*$"
)
GETTER_VECTOR_CREF_RE = re.compile(
    r"^\s*(?:\[\[[^\]]*\]\]\s+)*(?!static\b)(?:virtual\s+)?"
    r"const\s+std::vector<\s*" + _VEC_ELT + r"\s*>\s*&\s*"
    r"(\w+)\s*\([^)]*\)\s*(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\s*;\s*$"
)
GETTER_VECTOR_CONSTREF2_RE = re.compile(
    r"^\s*(?:\[\[[^\]]*\]\]\s+)*(?!static\b)(?:virtual\s+)?"
    r"std::vector<\s*" + _VEC_ELT + r"\s*>\s*const\s*&\s*"
    r"(\w+)\s*\([^)]*\)\s*(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\s*;\s*$"
)
# void Method(Type param); — one parameter, known value type (v1).
SETTER_METHOD_RE = re.compile(
    r"^\s*(?:\[\[[^\]]*\]\]\s+)*(?!static\b)(?:virtual\s+)?void\s+"
    r"(\w+)\s*\(\s*(?:const\s+)?"
    r"(float|double|bool|int|std::int32_t|std::int64_t|std::string|sf::Vector2f|sf::Vector3f|sf::Color)"
    r"(?:\s*&)?\s*\w*\s*\)\s*"
    r"(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\s*;\s*$"
)
SETTER_VECTOR_METHOD_RE = re.compile(
    r"^\s*(?:\[\[[^\]]*\]\]\s+)*(?!static\b)(?:virtual\s+)?void\s+"
    r"(\w+)\s*\(\s*(?:const\s+)?std::vector<\s*" + _VEC_ELT + r"\s*>\s*(?:const\s*)?(?:&)?\s*\w+\s*\)\s*"
    r"(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\s*;\s*$"
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
    is_getter: bool = False
    is_vector: bool = False
    is_bitset: bool = False
    is_map: bool = False
    is_set: bool = False
    map_value_type: str | None = None
    assoc_container: str | None = None


PendingKind = Literal["property", "getter", "setter"]


@dataclass
class GetterDecl:
    cpp_type: str
    method: str
    line: int
    col: int
    attrs: dict[str, Any]
    is_vector: bool = False


@dataclass
class SetterDecl:
    cpp_type: str
    method: str
    line: int
    col: int
    attrs: dict[str, Any]
    is_vector: bool = False


def pair_tag_key(attrs: dict[str, Any]) -> str | None:
    for k in ("name", "label"):
        v = attrs.get(k)
        if isinstance(v, str) and v.strip():
            return v.strip()
    return None


def merge_getter_setter_decls(
    path: Path,
    getters: list[GetterDecl],
    setters: list[SetterDecl],
) -> list[PropSpec]:
    if not getters and not setters:
        return []

    by_key: dict[str, SetterDecl] = {}
    for s in setters:
        key = pair_tag_key(s.attrs)
        if key is None:
            raise ParseError(
                '@setter requires name="..." or label="..." to pair with @getter',
                path,
                s.line,
                s.col,
            )
        if key in by_key:
            raise ParseError(f'duplicate @setter for name="{key}"', path, s.line, s.col)
        by_key[key] = s

    seen_gkeys: set[str] = set()
    out: list[PropSpec] = []

    for g in getters:
        gkey = pair_tag_key(g.attrs)
        if gkey is not None:
            if gkey in seen_gkeys:
                raise ParseError(f'duplicate @getter for name="{gkey}"', path, g.line, g.col)
            seen_gkeys.add(gkey)

        merged = dict(g.attrs)
        inline = g.attrs.get("setter")
        inline_ok = isinstance(inline, str) and bool(inline.strip())

        setter_method: str | None = None
        if inline_ok:
            setter_method = inline.strip()
            if gkey is not None and gkey in by_key:
                raise ParseError(
                    f'@getter uses both setter= and @setter with name="{gkey}"; use only one',
                    path,
                    g.line,
                    g.col,
                )
        elif gkey is not None and gkey in by_key:
            s = by_key.pop(gkey)
            if s.cpp_type != g.cpp_type or s.is_vector != g.is_vector:
                raise ParseError(
                    f'@getter / @setter pair "{gkey}": type mismatch ({g.cpp_type} vs {s.cpp_type})',
                    path,
                    g.line,
                    g.col,
                )
            setter_method = s.method

        if setter_method:
            merged["setter"] = setter_method
        else:
            merged.pop("setter", None)

        out.append(
            PropSpec(
                cpp_type=g.cpp_type,
                member=g.method,
                line=g.line,
                col=g.col,
                attrs=merged,
                is_getter=True,
                is_vector=g.is_vector,
                is_bitset=False,
                is_map=False,
                is_set=False,
                map_value_type=None,
                assoc_container=None,
            )
        )

    if by_key:
        s = next(iter(by_key.values()))
        k = pair_tag_key(s.attrs) or "?"
        raise ParseError(f'@setter name="{k}" has no matching @getter', path, s.line, s.col)

    return out


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
    pending: tuple[PendingKind, int, int, str] | None = None

    def close_class_block(b: dict[str, Any]) -> None:
        if not b.get("meta"):
            return
        props: list[PropSpec] = list(b.get("props", []))
        gdecls: list[GetterDecl] = list(b.get("getter_decls", []))
        sdecls: list[SetterDecl] = list(b.get("setter_decls", []))
        if gdecls or sdecls:
            props.extend(merge_getter_setter_decls(path, gdecls, sdecls))
        finished.append(
            ClassSpec(
                namespaces=tuple(b["namespaces"]),
                class_name=b["name"],
                props=props,
            )
        )

    for i, raw_line in enumerate(lines):
        line_no = i + 1
        line = strip_line_comment_keep_doc(raw_line)
        stripped = line.strip()

        if pending is not None:
            kind, pline, pcol, pargs = pending
            if not stripped or stripped.startswith("///"):
                continue
            if re.match(r"^\s*(public|private|protected)\s*:\s*$", line):
                continue
            inner = find_innermost_class(block_stack)
            if inner is None or not inner.get("meta"):
                tag = {"property": "@property", "getter": "@getter", "setter": "@setter"}[kind]
                raise ParseError(
                    f"{tag} must appear inside a class marked with META_CLASS()",
                    path,
                    pline,
                    pcol,
                )
            attrs = parse_attr_dict(pargs, path, pline, pcol)
            if kind == "property":
                m_vec = VECTOR_FIELD_RE.match(line)
                if m_vec:
                    cpp_type, member = m_vec.group(1), m_vec.group(2)
                    inner.setdefault("props", []).append(
                        PropSpec(
                            cpp_type=cpp_type,
                            member=member,
                            line=pline,
                            col=pcol,
                            attrs=attrs,
                            is_getter=False,
                            is_vector=True,
                            is_bitset=False,
                        )
                    )
                elif m_map := MAP_FIELD_RE.match(line):
                    cont, kt, vt, member = (
                        m_map.group(1),
                        m_map.group(2),
                        m_map.group(3),
                        m_map.group(4),
                    )
                    if kt not in KNOWN_TYPES or vt not in KNOWN_TYPES:
                        raise ParseError(
                            f"std::map key/value must be supported scalar types (tag at line {pline})",
                            path,
                            line_no,
                            1,
                        )
                    inner.setdefault("props", []).append(
                        PropSpec(
                            cpp_type=kt,
                            member=member,
                            line=pline,
                            col=pcol,
                            attrs=attrs,
                            is_getter=False,
                            is_vector=False,
                            is_bitset=False,
                            is_map=True,
                            is_set=False,
                            map_value_type=vt,
                            assoc_container=cont,
                        )
                    )
                elif m_set := SET_FIELD_RE.match(line):
                    cont, et, member = m_set.group(1), m_set.group(2), m_set.group(3)
                    if et not in KNOWN_TYPES:
                        raise ParseError(
                            f"std::set element must be a supported scalar type (tag at line {pline})",
                            path,
                            line_no,
                            1,
                        )
                    inner.setdefault("props", []).append(
                        PropSpec(
                            cpp_type=et,
                            member=member,
                            line=pline,
                            col=pcol,
                            attrs=attrs,
                            is_getter=False,
                            is_vector=False,
                            is_bitset=False,
                            is_map=False,
                            is_set=True,
                            map_value_type=None,
                            assoc_container=cont,
                        )
                    )
                elif (m_bs := BITSET_FIELD_RE.match(line)) or (
                    (m_td := BITSET_TYPEDEF_FIELD_RE.match(line))
                    and m_td.group(1) in (inner.get("bitset_aliases") or set())
                ):
                    member = (
                        m_bs.group(1)
                        if m_bs
                        else m_td.group(2)  # type: ignore[union-attr]
                    )
                    inner.setdefault("props", []).append(
                        PropSpec(
                            cpp_type="std::bitset",
                            member=member,
                            line=pline,
                            col=pcol,
                            attrs=attrs,
                            is_getter=False,
                            is_vector=False,
                            is_bitset=True,
                        )
                    )
                else:
                    m_field = FIELD_RE.match(line)
                    if not m_field:
                        raise ParseError(
                            f"expected field with supported type after @property (tag at line {pline})",
                            path,
                            line_no,
                            1,
                        )
                    cpp_type, member = m_field.group(1), m_field.group(2)
                    inner.setdefault("props", []).append(
                        PropSpec(
                            cpp_type=cpp_type,
                            member=member,
                            line=pline,
                            col=pcol,
                            attrs=attrs,
                            is_getter=False,
                            is_vector=False,
                            is_bitset=False,
                        )
                    )
            elif kind == "getter":
                if re.search(r"\bstatic\b", line):
                    raise ParseError(
                        f"@getter does not support static methods (tag at line {pline})",
                        path,
                        line_no,
                        1,
                    )
                m_getter = GETTER_METHOD_RE.match(line)
                if m_getter:
                    cpp_type, method = m_getter.group(1), m_getter.group(2)
                    inner.setdefault("getter_decls", []).append(
                        GetterDecl(
                            cpp_type=cpp_type,
                            method=method,
                            line=pline,
                            col=pcol,
                            attrs=attrs,
                            is_vector=False,
                        )
                    )
                elif (m_vec := GETTER_VECTOR_VAL_RE.match(line)) or (
                    m_vec := GETTER_VECTOR_CREF_RE.match(line)
                ) or (m_vec := GETTER_VECTOR_CONSTREF2_RE.match(line)):
                    cpp_type, method = m_vec.group(1), m_vec.group(2)
                    inner.setdefault("getter_decls", []).append(
                        GetterDecl(
                            cpp_type=cpp_type,
                            method=method,
                            line=pline,
                            col=pcol,
                            attrs=attrs,
                            is_vector=True,
                        )
                    )
                else:
                    raise ParseError(
                        f"expected instance method with supported return type after @getter (tag at line {pline})",
                        path,
                        line_no,
                        1,
                    )
            else:
                if re.search(r"\bstatic\b", line):
                    raise ParseError(
                        f"@setter does not support static methods (tag at line {pline})",
                        path,
                        line_no,
                        1,
                    )
                m_setter_vec = SETTER_VECTOR_METHOD_RE.match(line)
                if m_setter_vec:
                    smethod, stype = m_setter_vec.group(1), m_setter_vec.group(2)
                    inner.setdefault("setter_decls", []).append(
                        SetterDecl(
                            cpp_type=stype,
                            method=smethod,
                            line=pline,
                            col=pcol,
                            attrs=attrs,
                            is_vector=True,
                        )
                    )
                else:
                    m_setter = SETTER_METHOD_RE.match(line)
                    if not m_setter:
                        raise ParseError(
                            f"expected void Method(value_type) after @setter (tag at line {pline})",
                            path,
                            line_no,
                            1,
                        )
                    smethod, stype = m_setter.group(1), m_setter.group(2)
                    inner.setdefault("setter_decls", []).append(
                        SetterDecl(
                            cpp_type=stype,
                            method=smethod,
                            line=pline,
                            col=pcol,
                            attrs=attrs,
                            is_vector=False,
                        )
                    )
            pending = None
            continue

        m_using_bitset_alias = USING_BITSET_ALIAS_RE.match(line)
        if m_using_bitset_alias:
            inner_alias = find_innermost_class(block_stack)
            if inner_alias is not None:
                inner_alias.setdefault("bitset_aliases", set()).add(m_using_bitset_alias.group(1))

        m_prop = PROPERTY_TAG_RE.match(raw_line)
        if m_prop:
            if pending is not None:
                raise ParseError(
                    "unfinished @property, @getter, or @setter (a new tag started before the declaration line)",
                    path,
                    line_no,
                    1,
                )
            args = m_prop.group(1) or ""
            try:
                col = raw_line.index("@property") + 1
            except ValueError:
                col = 1
            pending = ("property", line_no, col, args)
            continue

        m_getter_tag = GETTER_TAG_RE.match(raw_line)
        if m_getter_tag:
            if pending is not None:
                raise ParseError(
                    "unfinished @property, @getter, or @setter (a new tag started before the declaration line)",
                    path,
                    line_no,
                    1,
                )
            args = m_getter_tag.group(1) or ""
            try:
                col = raw_line.index("@getter") + 1
            except ValueError:
                col = 1
            pending = ("getter", line_no, col, args)
            continue

        m_setter_tag = SETTER_TAG_RE.match(raw_line)
        if m_setter_tag:
            if pending is not None:
                raise ParseError(
                    "unfinished @property, @getter, or @setter (a new tag started before the declaration line)",
                    path,
                    line_no,
                    1,
                )
            args = m_setter_tag.group(1) or ""
            try:
                col = raw_line.index("@setter") + 1
            except ValueError:
                col = 1
            pending = ("setter", line_no, col, args)
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

    if pending:
        kind, pline, pcol, _ = pending
        tag = {"property": "@property", "getter": "@getter", "setter": "@setter"}[kind]
        raise ParseError(f"unfinished {tag} (no declaration line before EOF)", path, pline, pcol)

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
    has_setter = isinstance(a.get("setter"), str) and bool(a.get("setter"))
    ro = (a.get("readonly") is True) or (p.is_getter and not has_setter)
    parts: list[str] = ["Engine::PropertyMeta _m;"]
    if ro:
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
    if p.is_vector:
        for key, mk in (("minCount", "minElementCount"), ("maxCount", "maxElementCount")):
            if key not in a:
                continue
            v = a[key]
            if isinstance(v, (int, float)):
                parts.append(f"_m.{mk} = static_cast<std::size_t>({int(v)});")
    inner = " ".join(parts)
    return f"[&]() -> Engine::PropertyMeta {{ {inner} return _m; }}()"


def emit_bitset_property(
    out: list[str],
    p: PropSpec,
    meta_arg: str,
    readonly: bool,
) -> None:
    fid = member_to_field_id(p.member)
    label_esc = cpp_escape_string(default_label(p.member, p.attrs))
    idx = f"_pv_{fid}_i"
    mem = p.member
    size_body = f"{mem}.size()"

    seq = (
        "Engine::PropAccessSequence{\n"
        f"\t\t[this]() {{ return {size_body}; }},\n"
        "\t\t{}\n"
        "\t}"
    )
    out.append(f'\tb.beginSequence("{fid}", "{label_esc}", {seq}, {meta_arg});')
    out.append(f"\tfor (std::size_t {idx} = 0; {idx} < {size_body}; ++{idx}) {{")
    out.append(
        f'\t\tb.pushObject(std::to_string({idx}), "[" + std::to_string({idx}) + "]", Engine::PropertyMeta{{}});'
    )

    get_bit = f"{mem}.test({idx})"
    g = f"[this, {idx}]() {{ return {get_bit}; }}"
    if readonly:
        sl = f"[this, {idx}](bool) {{}}"
        out.append(f'\t\tb.addBool("v", "", {g}, {sl}, {meta_arg});')
    else:
        wl = f"[this, {idx}](bool v) {{ if (v) {mem}.set({idx}); else {mem}.reset({idx}); }}"
        out.append(f'\t\tb.addBool("v", "", {g}, {wl}, {meta_arg});')

    out.append("\t\tb.pop();")
    out.append("\t}")
    out.append("\tb.endSequence();")


def _default_cpp_value(t: str) -> str:
    return {
        "float": "0.f",
        "double": "0.",
        "bool": "false",
        "int": "0",
        "std::int32_t": "static_cast<std::int32_t>(0)",
        "std::int64_t": "static_cast<std::int64_t>(0)",
        "std::string": "std::string{}",
        "sf::Vector2f": "sf::Vector2f{}",
        "sf::Vector3f": "sf::Vector3f{}",
        "sf::Color": "sf::Color{}",
    }[t]


def _int32_param_cpp(t: str) -> str:
    return "std::int32_t" if t == "int" else t


def _emit_scalar_leaf(
    out: list[str],
    t: str,
    nid: str,
    label_esc: str,
    get_l: str,
    set_l: str,
    meta: str,
    path: Path,
    line: int,
    col: int,
) -> None:
    if t == "float":
        out.append(f'\t\tb.addFloat("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "double":
        out.append(f'\t\tb.addDouble("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "bool":
        out.append(f'\t\tb.addBool("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t in ("int", "std::int32_t"):
        out.append(f'\t\tb.addInt32("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "std::int64_t":
        out.append(f'\t\tb.addInt64("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "std::string":
        out.append(f'\t\tb.addString("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "sf::Vector2f":
        out.append(f'\t\tb.addVec2f("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "sf::Vector3f":
        out.append(f'\t\tb.addVec3f("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "sf::Color":
        out.append(f'\t\tb.addColor("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    else:
        raise ParseError(f"unsupported scalar type `{t}`", path, line, col)


def _map_key_get(mem: str, idx: str, kt: str) -> str:
    adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
    if kt == "int":
        return f"[this, {idx}]() {{ {adv}return static_cast<std::int32_t>(_it->first); }}"
    return f"[this, {idx}]() {{ {adv}return _it->first; }}"


def _map_key_set(mem: str, idx: str, kt: str, readonly: bool) -> str:
    if readonly:
        ro: dict[str, str] = {
            "float": f"[this, {idx}](float) {{}}",
            "double": f"[this, {idx}](double) {{}}",
            "bool": f"[this, {idx}](bool) {{}}",
            "int": f"[this, {idx}](std::int32_t) {{}}",
            "std::int32_t": f"[this, {idx}](std::int32_t) {{}}",
            "std::int64_t": f"[this, {idx}](std::int64_t) {{}}",
            "std::string": f"[this, {idx}](std::string) {{}}",
            "sf::Vector2f": f"[this, {idx}](sf::Vector2f) {{}}",
            "sf::Vector3f": f"[this, {idx}](sf::Vector3f) {{}}",
            "sf::Color": f"[this, {idx}](sf::Color) {{}}",
        }
        return ro[kt]
    adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
    mv = "auto _mapped = std::move(_it->second); " + f"{mem}.erase(_it); "
    pk = _int32_param_cpp(kt)
    if kt == "int":
        ins = f"{mem}.insert_or_assign(static_cast<int>(newKey), std::move(_mapped))"
    elif kt == "std::string":
        ins = f"{mem}.insert_or_assign(std::move(newKey), std::move(_mapped))"
    elif kt in ("sf::Vector2f", "sf::Vector3f", "sf::Color"):
        ins = f"{mem}.insert_or_assign(std::move(newKey), std::move(_mapped))"
    else:
        ins = f"{mem}.insert_or_assign(newKey, std::move(_mapped))"
    return f"[this, {idx}]({pk} newKey) {{ {adv}{mv}{ins}; }}"


def _map_val_get(mem: str, idx: str, vt: str) -> str:
    adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
    if vt == "int":
        return f"[this, {idx}]() {{ {adv}return static_cast<std::int32_t>(_it->second); }}"
    return f"[this, {idx}]() {{ {adv}return _it->second; }}"


def _map_val_set(mem: str, idx: str, vt: str, readonly: bool) -> str:
    if readonly:
        ro: dict[str, str] = {
            "float": f"[this, {idx}](float) {{}}",
            "double": f"[this, {idx}](double) {{}}",
            "bool": f"[this, {idx}](bool) {{}}",
            "int": f"[this, {idx}](std::int32_t) {{}}",
            "std::int32_t": f"[this, {idx}](std::int32_t) {{}}",
            "std::int64_t": f"[this, {idx}](std::int64_t) {{}}",
            "std::string": f"[this, {idx}](std::string) {{}}",
            "sf::Vector2f": f"[this, {idx}](sf::Vector2f) {{}}",
            "sf::Vector3f": f"[this, {idx}](sf::Vector3f) {{}}",
            "sf::Color": f"[this, {idx}](sf::Color) {{}}",
        }
        return ro[vt]
    adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
    if vt == "int":
        return f"[this, {idx}](std::int32_t newVal) {{ {adv}_it->second = static_cast<int>(newVal); }}"
    if vt == "std::string":
        return f"[this, {idx}](std::string newVal) {{ {adv}_it->second = std::move(newVal); }}"
    if vt in ("sf::Vector2f", "sf::Vector3f", "sf::Color"):
        return f"[this, {idx}]({vt} newVal) {{ {adv}_it->second = std::move(newVal); }}"
    return f"[this, {idx}]({vt} newVal) {{ {adv}_it->second = newVal; }}"


def _cpp_map_emplace(mem: str, kt: str, key_var: str, dv: str) -> str:
    if kt == "std::string":
        return f"{mem}.emplace(std::move({key_var}), {dv})"
    if kt in ("sf::Vector2f", "sf::Vector3f", "sf::Color"):
        return f"{mem}.emplace(std::move({key_var}), {dv})"
    return f"{mem}.emplace({key_var}, {dv})"


def _cpp_set_insert(mem: str, et: str, val_var: str) -> str:
    if et == "std::string":
        return f"{mem}.insert(std::move({val_var}))"
    if et in ("sf::Vector2f", "sf::Vector3f", "sf::Color"):
        return f"{mem}.insert(std::move({val_var}))"
    return f"{mem}.insert({val_var})"


def _map_add_pair_body(mem: str, kt: str, vt: str, path: Path, line: int, col: int) -> str:
    """Statements for addPair: choose a key not already present, then emplace."""
    dv = _default_cpp_value(vt)
    tab = "\t\t\t"
    if kt in ("int", "std::int32_t"):
        decl = "int _nk = 0;" if kt == "int" else "std::int32_t _nk = 0;"
        return (
            f"{tab}{decl}\n"
            f"{tab}while ({mem}.find(_nk) != {mem}.end()) {{\n"
            f"{tab}\t++_nk;\n"
            f"{tab}}}\n"
            f"{tab}{_cpp_map_emplace(mem, kt, '_nk', dv)};"
        )
    if kt == "std::int64_t":
        return (
            f"{tab}std::int64_t _nk = 0;\n"
            f"{tab}while ({mem}.find(_nk) != {mem}.end()) {{\n"
            f"{tab}\t++_nk;\n"
            f"{tab}}}\n"
            f"{tab}{_cpp_map_emplace(mem, kt, '_nk', dv)};"
        )
    if kt == "bool":
        return (
            f"{tab}if ({mem}.find(false) == {mem}.end()) {{\n"
            f"{tab}\t{_cpp_map_emplace(mem, kt, 'false', dv)};\n"
            f"{tab}}} else if ({mem}.find(true) == {mem}.end()) {{\n"
            f"{tab}\t{_cpp_map_emplace(mem, kt, 'true', dv)};\n"
            f"{tab}}}"
        )
    if kt == "float":
        return (
            f"{tab}{{\n"
            f"{tab}\tfloat _nk = 0.f;\n"
            f"{tab}\tfor (int _g = 0; _g < 1000000 && {mem}.find(_nk) != {mem}.end(); ++_g) {{\n"
            f"{tab}\t\t_nk += 1.f;\n"
            f"{tab}\t}}\n"
            f"{tab}\tif ({mem}.find(_nk) == {mem}.end()) {{\n"
            f"{tab}\t\t{_cpp_map_emplace(mem, kt, '_nk', dv)};\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    if kt == "double":
        return (
            f"{tab}{{\n"
            f"{tab}\tdouble _nk = 0.;\n"
            f"{tab}\tfor (int _g = 0; _g < 1000000 && {mem}.find(_nk) != {mem}.end(); ++_g) {{\n"
            f"{tab}\t\t_nk += 1.;\n"
            f"{tab}\t}}\n"
            f"{tab}\tif ({mem}.find(_nk) == {mem}.end()) {{\n"
            f"{tab}\t\t{_cpp_map_emplace(mem, kt, '_nk', dv)};\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    if kt == "std::string":
        return (
            f"{tab}{{\n"
            f"{tab}\tstd::size_t _i = {mem}.size();\n"
            f"{tab}\tstd::string _nk;\n"
            f"{tab}\tdo {{\n"
            f'{tab}\t\t_nk = std::string("__new_") + std::to_string(_i++);\n'
            f"{tab}\t}} while ({mem}.find(_nk) != {mem}.end());\n"
            f"{tab}\t{_cpp_map_emplace(mem, kt, '_nk', dv)};\n"
            f"{tab}}}"
        )
    if kt == "sf::Vector2f":
        return (
            f"{tab}{{\n"
            f"{tab}\tbool _placed = false;\n"
            f"{tab}\tfor (int _ox = 0; _ox < 100000 && !_placed; ++_ox) {{\n"
            f"{tab}\t\tsf::Vector2f _nk{{static_cast<float>(_ox), 0.f}};\n"
            f"{tab}\t\tif ({mem}.find(_nk) == {mem}.end()) {{\n"
            f"{tab}\t\t\t{_cpp_map_emplace(mem, kt, '_nk', dv)};\n"
            f"{tab}\t\t\t_placed = true;\n"
            f"{tab}\t\t}}\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    if kt == "sf::Vector3f":
        return (
            f"{tab}{{\n"
            f"{tab}\tbool _placed = false;\n"
            f"{tab}\tfor (int _ox = 0; _ox < 100000 && !_placed; ++_ox) {{\n"
            f"{tab}\t\tsf::Vector3f _nk{{static_cast<float>(_ox), 0.f, 0.f}};\n"
            f"{tab}\t\tif ({mem}.find(_nk) == {mem}.end()) {{\n"
            f"{tab}\t\t\t{_cpp_map_emplace(mem, kt, '_nk', dv)};\n"
            f"{tab}\t\t\t_placed = true;\n"
            f"{tab}\t\t}}\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    if kt == "sf::Color":
        return (
            f"{tab}{{\n"
            f"{tab}\tbool _placed = false;\n"
            f"{tab}\tfor (unsigned _n = 0; _n < 0xFFFFFFu && !_placed; ++_n) {{\n"
            f"{tab}\t\tsf::Color _nk{{\n"
            f"{tab}\t\t\tstatic_cast<std::uint8_t>(_n & 0xFFu),\n"
            f"{tab}\t\t\tstatic_cast<std::uint8_t>((_n >> 8) & 0xFFu),\n"
            f"{tab}\t\t\tstatic_cast<std::uint8_t>((_n >> 16) & 0xFFu),\n"
            f"{tab}\t\t\t255}};\n"
            f"{tab}\t\tif ({mem}.find(_nk) == {mem}.end()) {{\n"
            f"{tab}\t\t\t{_cpp_map_emplace(mem, kt, '_nk', dv)};\n"
            f"{tab}\t\t\t_placed = true;\n"
            f"{tab}\t\t}}\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    raise ParseError(f"map addPair: unsupported key type `{kt}`", path, line, col)


def _set_add_pair_body(mem: str, et: str, path: Path, line: int, col: int) -> str:
    """Statements for addPair on a set: choose a value not already present, then insert."""
    tab = "\t\t\t"
    if et in ("int", "std::int32_t"):
        decl = "int _nv = 0;" if et == "int" else "std::int32_t _nv = 0;"
        return (
            f"{tab}{decl}\n"
            f"{tab}while ({mem}.find(_nv) != {mem}.end()) {{\n"
            f"{tab}\t++_nv;\n"
            f"{tab}}}\n"
            f"{tab}{_cpp_set_insert(mem, et, '_nv')};"
        )
    if et == "std::int64_t":
        return (
            f"{tab}std::int64_t _nv = 0;\n"
            f"{tab}while ({mem}.find(_nv) != {mem}.end()) {{\n"
            f"{tab}\t++_nv;\n"
            f"{tab}}}\n"
            f"{tab}{_cpp_set_insert(mem, et, '_nv')};"
        )
    if et == "bool":
        return (
            f"{tab}if ({mem}.find(false) == {mem}.end()) {{\n"
            f"{tab}\t{_cpp_set_insert(mem, et, 'false')};\n"
            f"{tab}}} else if ({mem}.find(true) == {mem}.end()) {{\n"
            f"{tab}\t{_cpp_set_insert(mem, et, 'true')};\n"
            f"{tab}}}"
        )
    if et == "float":
        return (
            f"{tab}{{\n"
            f"{tab}\tfloat _nv = 0.f;\n"
            f"{tab}\tfor (int _g = 0; _g < 1000000 && {mem}.find(_nv) != {mem}.end(); ++_g) {{\n"
            f"{tab}\t\t_nv += 1.f;\n"
            f"{tab}\t}}\n"
            f"{tab}\tif ({mem}.find(_nv) == {mem}.end()) {{\n"
            f"{tab}\t\t{_cpp_set_insert(mem, et, '_nv')};\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    if et == "double":
        return (
            f"{tab}{{\n"
            f"{tab}\tdouble _nv = 0.;\n"
            f"{tab}\tfor (int _g = 0; _g < 1000000 && {mem}.find(_nv) != {mem}.end(); ++_g) {{\n"
            f"{tab}\t\t_nv += 1.;\n"
            f"{tab}\t}}\n"
            f"{tab}\tif ({mem}.find(_nv) == {mem}.end()) {{\n"
            f"{tab}\t\t{_cpp_set_insert(mem, et, '_nv')};\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    if et == "std::string":
        return (
            f"{tab}{{\n"
            f"{tab}\tstd::size_t _i = {mem}.size();\n"
            f"{tab}\tstd::string _nv;\n"
            f"{tab}\tdo {{\n"
            f'{tab}\t\t_nv = std::string("__new_") + std::to_string(_i++);\n'
            f"{tab}\t}} while ({mem}.find(_nv) != {mem}.end());\n"
            f"{tab}\t{_cpp_set_insert(mem, et, '_nv')};\n"
            f"{tab}}}"
        )
    if et == "sf::Vector2f":
        return (
            f"{tab}{{\n"
            f"{tab}\tbool _placed = false;\n"
            f"{tab}\tfor (int _ox = 0; _ox < 100000 && !_placed; ++_ox) {{\n"
            f"{tab}\t\tsf::Vector2f _nv{{static_cast<float>(_ox), 0.f}};\n"
            f"{tab}\t\tif ({mem}.find(_nv) == {mem}.end()) {{\n"
            f"{tab}\t\t\t{_cpp_set_insert(mem, et, '_nv')};\n"
            f"{tab}\t\t\t_placed = true;\n"
            f"{tab}\t\t}}\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    if et == "sf::Vector3f":
        return (
            f"{tab}{{\n"
            f"{tab}\tbool _placed = false;\n"
            f"{tab}\tfor (int _ox = 0; _ox < 100000 && !_placed; ++_ox) {{\n"
            f"{tab}\t\tsf::Vector3f _nv{{static_cast<float>(_ox), 0.f, 0.f}};\n"
            f"{tab}\t\tif ({mem}.find(_nv) == {mem}.end()) {{\n"
            f"{tab}\t\t\t{_cpp_set_insert(mem, et, '_nv')};\n"
            f"{tab}\t\t\t_placed = true;\n"
            f"{tab}\t\t}}\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    if et == "sf::Color":
        return (
            f"{tab}{{\n"
            f"{tab}\tbool _placed = false;\n"
            f"{tab}\tfor (unsigned _n = 0; _n < 0xFFFFFFu && !_placed; ++_n) {{\n"
            f"{tab}\t\tsf::Color _nv{{\n"
            f"{tab}\t\t\tstatic_cast<std::uint8_t>(_n & 0xFFu),\n"
            f"{tab}\t\t\tstatic_cast<std::uint8_t>((_n >> 8) & 0xFFu),\n"
            f"{tab}\t\t\tstatic_cast<std::uint8_t>((_n >> 16) & 0xFFu),\n"
            f"{tab}\t\t\t255}};\n"
            f"{tab}\t\tif ({mem}.find(_nv) == {mem}.end()) {{\n"
            f"{tab}\t\t\t{_cpp_set_insert(mem, et, '_nv')};\n"
            f"{tab}\t\t\t_placed = true;\n"
            f"{tab}\t\t}}\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    raise ParseError(f"set addPair: unsupported element type `{et}`", path, line, col)


def emit_assoc_map_property(
    out: list[str],
    p: PropSpec,
    path: Path,
    meta_arg: str,
    readonly: bool,
) -> None:
    vt = p.map_value_type
    if vt is None:
        raise ParseError("internal: map without map_value_type", path, p.line, p.col)
    kt = p.cpp_type
    mem = p.member
    fid = member_to_field_id(p.member)
    label_esc = cpp_escape_string(default_label(p.member, p.attrs))
    idx = f"_pv_{fid}_i"
    leaf_meta = "Engine::PropertyMeta{}"

    if readonly:
        asc = "Engine::PropAccessAssociative{\n\t\t{},\n\t\t{}\n\t}"
    else:
        add_body = _map_add_pair_body(mem, kt, vt, path, p.line, p.col)
        asc = (
            "Engine::PropAccessAssociative{\n"
            f"\t\t[this]() {{\n{add_body}\n\t\t}},\n"
            "\t\t[this](std::size_t pairIndex) {\n"
            f"\t\t\tauto _it = {mem}.begin();\n"
            "\t\t\tstd::advance(_it, pairIndex);\n"
            f"\t\t\t{mem}.erase(_it);\n"
            "\t\t}\n"
            "\t}"
        )
    out.append(f'\tb.beginAssociative("{fid}", "{label_esc}", {asc}, {meta_arg});')
    out.append(f"\tfor (std::size_t {idx} = 0; {idx} < {mem}.size(); ++{idx}) {{")
    out.append(
        f'\t\tb.pushObject(std::to_string({idx}), "[" + std::to_string({idx}) + "]", Engine::PropertyMeta{{}});'
    )
    gk, sk = _map_key_get(mem, idx, kt), _map_key_set(mem, idx, kt, readonly)
    gv, sv = _map_val_get(mem, idx, vt), _map_val_set(mem, idx, vt, readonly)
    _emit_scalar_leaf(out, kt, "key", "Key", gk, sk, leaf_meta, path, p.line, p.col)
    _emit_scalar_leaf(out, vt, "value", "Value", gv, sv, leaf_meta, path, p.line, p.col)
    out.append("\t\tb.pop();")
    out.append("\t}")
    out.append("\tb.endAssociative();")


def emit_assoc_set_property(
    out: list[str],
    p: PropSpec,
    path: Path,
    meta_arg: str,
    readonly: bool,
) -> None:
    et = p.cpp_type
    mem = p.member
    fid = member_to_field_id(p.member)
    label_esc = cpp_escape_string(default_label(p.member, p.attrs))
    idx = f"_pv_{fid}_i"
    leaf_meta = "Engine::PropertyMeta{}"

    if readonly:
        asc = "Engine::PropAccessAssociative{\n\t\t{},\n\t\t{}\n\t}"
    else:
        add_body = _set_add_pair_body(mem, et, path, p.line, p.col)
        asc = (
            "Engine::PropAccessAssociative{\n"
            f"\t\t[this]() {{\n{add_body}\n\t\t}},\n"
            "\t\t[this](std::size_t pairIndex) {\n"
            f"\t\t\tauto _it = {mem}.begin();\n"
            "\t\t\tstd::advance(_it, pairIndex);\n"
            f"\t\t\t{mem}.erase(_it);\n"
            "\t\t}\n"
            "\t}"
        )
    out.append(f'\tb.beginAssociative("{fid}", "{label_esc}", {asc}, {meta_arg});')
    out.append(f"\tfor (std::size_t {idx} = 0; {idx} < {mem}.size(); ++{idx}) {{")
    out.append(
        f'\t\tb.pushObject(std::to_string({idx}), "[" + std::to_string({idx}) + "]", Engine::PropertyMeta{{}});'
    )
    if et == "int":
        g = f"[this, {idx}]() {{ auto _it = {mem}.begin(); std::advance(_it, {idx}); return static_cast<std::int32_t>(*_it); }}"
    else:
        g = f"[this, {idx}]() {{ auto _it = {mem}.begin(); std::advance(_it, {idx}); return *_it; }}"
    if readonly:
        if et in ("int", "std::int32_t"):
            s = f"[this, {idx}](std::int32_t) {{}}"
        else:
            s = f"[this, {idx}]({et}) {{}}"
    else:
        adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); {mem}.erase(_it); "
        if et == "int":
            s = f"[this, {idx}](std::int32_t v) {{ {adv}{mem}.insert(static_cast<int>(v)); }}"
        elif et == "std::string":
            s = f"[this, {idx}](std::string v) {{ {adv}{mem}.insert(std::move(v)); }}"
        elif et in ("sf::Vector2f", "sf::Vector3f", "sf::Color"):
            s = f"[this, {idx}]({et} v) {{ {adv}{mem}.insert(std::move(v)); }}"
        else:
            s = f"[this, {idx}]({et} v) {{ {adv}{mem}.insert(v); }}"
    _emit_scalar_leaf(out, et, "v", "", g, s, leaf_meta, path, p.line, p.col)
    out.append("\t\tb.pop();")
    out.append("\t}")
    out.append("\tb.endAssociative();")


def emit_std_vector_property(
    out: list[str],
    p: PropSpec,
    path: Path,
    meta_arg: str,
    readonly: bool,
) -> None:
    setter_raw = p.attrs.get("setter")
    setter_name = setter_raw.strip() if isinstance(setter_raw, str) else ""

    if p.is_getter and not readonly and not setter_name:
        raise ParseError(
            "writable @getter returning std::vector requires a paired @setter (or setter= on @getter)",
            path,
            p.line,
            p.col,
        )

    fid = member_to_field_id(p.member)
    label_esc = cpp_escape_string(default_label(p.member, p.attrs))
    idx = f"_pv_{fid}_i"

    if p.is_getter:
        vec_access = f"this->{p.member}()"
    else:
        mem = p.member
        vec_access = mem

    size_body = f"{vec_access}.size()"

    if readonly:
        seq = (
            "Engine::PropAccessSequence{\n"
            f"\t\t[this]() {{ return {size_body}; }},\n"
            "\t\t{}\n"
            "\t}"
        )
    elif p.is_getter:
        seq = (
            "Engine::PropAccessSequence{\n"
            f"\t\t[this]() {{ return {size_body}; }},\n"
            f"\t\t[this](std::size_t _n) {{\n"
            f"\t\t\tauto _pv = {vec_access};\n"
            f"\t\t\t_pv.resize(_n);\n"
            f"\t\t\tthis->{setter_name}(std::move(_pv));\n"
            "\t\t}\n"
            "\t}"
        )
    else:
        seq = (
            "Engine::PropAccessSequence{\n"
            f"\t\t[this]() {{ return {size_body}; }},\n"
            f"\t\t[this](std::size_t _n) {{ {mem}.resize(_n); }}\n"
            "\t}"
        )
    out.append(f'\tb.beginSequence("{fid}", "{label_esc}", {seq}, {meta_arg});')
    out.append(f"\tfor (std::size_t {idx} = 0; {idx} < {size_body}; ++{idx}) {{")
    out.append(
        f'\t\tb.pushObject(std::to_string({idx}), "[" + std::to_string({idx}) + "]", Engine::PropertyMeta{{}});'
    )

    if p.is_getter:
        acc = f"{vec_access}[{idx}]"
    else:
        acc = f"{mem}[{idx}]"

    g = f"[this, {idx}] {{ return {acc}; }}"

    def add_inner(call: str) -> None:
        out.append(f"\t\t{call}")

    t = p.cpp_type
    if readonly:
        ro = {
            "float": f"[this, {idx}](float) {{}}",
            "double": f"[this, {idx}](double) {{}}",
            "bool": f"[this, {idx}](bool) {{}}",
            "int": f"[this, {idx}](std::int32_t) {{}}",
            "std::int32_t": f"[this, {idx}](std::int32_t) {{}}",
            "std::int64_t": f"[this, {idx}](std::int64_t) {{}}",
            "std::string": f"[this, {idx}](std::string) {{}}",
            "sf::Vector2f": f"[this, {idx}](sf::Vector2f) {{}}",
            "sf::Vector3f": f"[this, {idx}](sf::Vector3f) {{}}",
            "sf::Color": f"[this, {idx}](sf::Color) {{}}",
        }
        sl = ro[t]
        if t == "float":
            add_inner(f'b.addFloat("v", "", {g}, {sl}, {meta_arg});')
        elif t == "double":
            add_inner(f'b.addDouble("v", "", {g}, {sl}, {meta_arg});')
        elif t == "bool":
            add_inner(f'b.addBool("v", "", {g}, {sl}, {meta_arg});')
        elif t in ("int", "std::int32_t"):
            add_inner(f'b.addInt32("v", "", {g}, {sl}, {meta_arg});')
        elif t == "std::int64_t":
            add_inner(f'b.addInt64("v", "", {g}, {sl}, {meta_arg});')
        elif t == "std::string":
            add_inner(f'b.addString("v", "", {g}, {sl}, {meta_arg});')
        elif t == "sf::Vector2f":
            add_inner(f'b.addVec2f("v", "", {g}, {sl}, {meta_arg});')
        elif t == "sf::Vector3f":
            add_inner(f'b.addVec3f("v", "", {g}, {sl}, {meta_arg});')
        elif t == "sf::Color":
            add_inner(f'b.addColor("v", "", {g}, {sl}, {meta_arg});')
        else:
            raise ParseError(f"unsupported vector element type `{t}`", path, p.line, p.col)
    elif p.is_getter:
        wl_vec = {
            "float": f"[this, {idx}](float v) {{ auto _pv = {vec_access}; _pv[{idx}] = v; this->{setter_name}(std::move(_pv)); }}",
            "double": f"[this, {idx}](double v) {{ auto _pv = {vec_access}; _pv[{idx}] = v; this->{setter_name}(std::move(_pv)); }}",
            "bool": f"[this, {idx}](bool v) {{ auto _pv = {vec_access}; _pv[{idx}] = v; this->{setter_name}(std::move(_pv)); }}",
            "int": f"[this, {idx}](std::int32_t v) {{ auto _pv = {vec_access}; _pv[{idx}] = static_cast<int>(v); this->{setter_name}(std::move(_pv)); }}",
            "std::int32_t": f"[this, {idx}](std::int32_t v) {{ auto _pv = {vec_access}; _pv[{idx}] = v; this->{setter_name}(std::move(_pv)); }}",
            "std::int64_t": f"[this, {idx}](std::int64_t v) {{ auto _pv = {vec_access}; _pv[{idx}] = v; this->{setter_name}(std::move(_pv)); }}",
            "std::string": f"[this, {idx}](std::string v) {{ auto _pv = {vec_access}; _pv[{idx}] = std::move(v); this->{setter_name}(std::move(_pv)); }}",
            "sf::Vector2f": f"[this, {idx}](sf::Vector2f v) {{ auto _pv = {vec_access}; _pv[{idx}] = v; this->{setter_name}(std::move(_pv)); }}",
            "sf::Vector3f": f"[this, {idx}](sf::Vector3f v) {{ auto _pv = {vec_access}; _pv[{idx}] = v; this->{setter_name}(std::move(_pv)); }}",
            "sf::Color": f"[this, {idx}](sf::Color c) {{ auto _pv = {vec_access}; _pv[{idx}] = c; this->{setter_name}(std::move(_pv)); }}",
        }
        wl = wl_vec[t]
        if t == "float":
            add_inner(f'b.addFloat("v", "", {g}, {wl}, {meta_arg});')
        elif t == "double":
            add_inner(f'b.addDouble("v", "", {g}, {wl}, {meta_arg});')
        elif t == "bool":
            add_inner(f'b.addBool("v", "", {g}, {wl}, {meta_arg});')
        elif t in ("int", "std::int32_t"):
            add_inner(f'b.addInt32("v", "", {g}, {wl}, {meta_arg});')
        elif t == "std::int64_t":
            add_inner(f'b.addInt64("v", "", {g}, {wl}, {meta_arg});')
        elif t == "std::string":
            add_inner(f'b.addString("v", "", {g}, {wl}, {meta_arg});')
        elif t == "sf::Vector2f":
            add_inner(f'b.addVec2f("v", "", {g}, {wl}, {meta_arg});')
        elif t == "sf::Vector3f":
            add_inner(f'b.addVec3f("v", "", {g}, {wl}, {meta_arg});')
        elif t == "sf::Color":
            add_inner(f'b.addColor("v", "", {g}, {wl}, {meta_arg});')
        else:
            raise ParseError(f"unsupported vector element type `{t}`", path, p.line, p.col)
    else:
        rw = {
            "float": f"[this, {idx}](float v) {{ {acc} = v; }}",
            "double": f"[this, {idx}](double v) {{ {acc} = v; }}",
            "bool": f"[this, {idx}](bool v) {{ {acc} = v; }}",
            "int": f"[this, {idx}](std::int32_t v) {{ {acc} = static_cast<int>(v); }}",
            "std::int32_t": f"[this, {idx}](std::int32_t v) {{ {acc} = v; }}",
            "std::int64_t": f"[this, {idx}](std::int64_t v) {{ {acc} = v; }}",
            "std::string": f"[this, {idx}](std::string v) {{ {acc} = std::move(v); }}",
            "sf::Vector2f": f"[this, {idx}](sf::Vector2f v) {{ {acc} = v; }}",
            "sf::Vector3f": f"[this, {idx}](sf::Vector3f v) {{ {acc} = v; }}",
            "sf::Color": f"[this, {idx}](sf::Color c) {{ {acc} = c; }}",
        }
        wl = rw[t]
        if t == "float":
            add_inner(f'b.addFloat("v", "", {g}, {wl}, {meta_arg});')
        elif t == "double":
            add_inner(f'b.addDouble("v", "", {g}, {wl}, {meta_arg});')
        elif t == "bool":
            add_inner(f'b.addBool("v", "", {g}, {wl}, {meta_arg});')
        elif t in ("int", "std::int32_t"):
            add_inner(f'b.addInt32("v", "", {g}, {wl}, {meta_arg});')
        elif t == "std::int64_t":
            add_inner(f'b.addInt64("v", "", {g}, {wl}, {meta_arg});')
        elif t == "std::string":
            add_inner(f'b.addString("v", "", {g}, {wl}, {meta_arg});')
        elif t == "sf::Vector2f":
            add_inner(f'b.addVec2f("v", "", {g}, {wl}, {meta_arg});')
        elif t == "sf::Vector3f":
            add_inner(f'b.addVec3f("v", "", {g}, {wl}, {meta_arg});')
        elif t == "sf::Color":
            add_inner(f'b.addColor("v", "", {g}, {wl}, {meta_arg});')
        else:
            raise ParseError(f"unsupported vector element type `{t}`", path, p.line, p.col)

    out.append("\t\tb.pop();")
    out.append("\t}")
    out.append("\tb.endSequence();")


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
    def uses_sfml(p: PropSpec) -> bool:
        if p.is_map:
            mv = p.map_value_type or ""
            return p.cpp_type.startswith("sf::") or mv.startswith("sf::")
        if p.is_set:
            return p.cpp_type.startswith("sf::")
        return p.cpp_type.startswith("sf::")

    needs_sfml = any(uses_sfml(p) for c in classes for p in c.props)
    if needs_sfml:
        out.append('#include <SFML/Graphics/Color.hpp>')
        out.append('#include <SFML/System/Vector2.hpp>')
        out.append('#include <SFML/System/Vector3.hpp>')
        out.append("")
    out.append("#include <cstdint>")
    out.append("#include <string>")
    if any(p.is_vector for c in classes for p in c.props):
        out.append("#include <cstddef>")
    if any(p.is_bitset for c in classes for p in c.props):
        out.append("#include <bitset>")
    assoc_headers: set[str] = set()
    for c in classes:
        for p in c.props:
            if not (p.is_map or p.is_set):
                continue
            ac = p.assoc_container
            if ac == "std::map":
                assoc_headers.add("<map>")
            elif ac == "std::unordered_map":
                assoc_headers.add("<unordered_map>")
            elif ac == "std::set":
                assoc_headers.add("<set>")
            elif ac == "std::unordered_set":
                assoc_headers.add("<unordered_set>")
    for h in sorted(assoc_headers):
        out.append(f"#include {h}")
    if assoc_headers:
        out.append("#include <iterator>")
    out.append("")

    for c in classes:
        q = c.qualified()
        root_id = pascal_to_snake(c.class_name)
        root_label = cpp_escape_string(c.class_name)
        out.append(f"void {q}::BuildPropertyTree(Engine::PropertyBuilder& b) {{")
        out.append(f'\tb.pushObject("{root_id}", "{root_label}");')
        for p in c.props:
            a = p.attrs
            setter_name = a.get("setter")
            has_setter_method = isinstance(setter_name, str) and bool(setter_name)
            readonly = (a.get("readonly") is True) or (p.is_getter and not has_setter_method)

            if p.is_bitset:
                if p.is_getter:
                    raise ParseError(
                        "std::bitset properties must be data members (not @getter) in codegen v1",
                        path,
                        p.line,
                        p.col,
                    )
                has_meta = readonly or any(
                    k in a for k in ("tooltip", "minValue", "maxValue", "step", "dragSpeed")
                )
                meta_arg = "Engine::PropertyMeta{}" if not has_meta else format_meta_inline(p)
                emit_bitset_property(out, p, meta_arg, readonly)
                continue

            if p.is_map or p.is_set:
                if p.is_getter:
                    raise ParseError(
                        "std::map / std::set fields must be data members (not @getter) in codegen v1",
                        path,
                        p.line,
                        p.col,
                    )
                has_meta = readonly or any(
                    k in a for k in ("tooltip", "minValue", "maxValue", "step", "dragSpeed")
                )
                meta_arg = "Engine::PropertyMeta{}" if not has_meta else format_meta_inline(p)
                if p.is_map:
                    emit_assoc_map_property(out, p, path, meta_arg, readonly)
                else:
                    emit_assoc_set_property(out, p, path, meta_arg, readonly)
                continue

            if p.cpp_type not in KNOWN_TYPES:
                raise ParseError(f"unsupported type `{p.cpp_type}`", path, p.line, p.col)

            if p.is_vector:
                has_meta = readonly or any(
                    k in a
                    for k in (
                        "tooltip",
                        "minValue",
                        "maxValue",
                        "step",
                        "dragSpeed",
                        "minCount",
                        "maxCount",
                    )
                )
                meta_arg = "Engine::PropertyMeta{}" if not has_meta else format_meta_inline(p)
                emit_std_vector_property(out, p, path, meta_arg, readonly)
                continue

            has_meta = readonly or any(
                k in a for k in ("tooltip", "minValue", "maxValue", "step", "dragSpeed")
            )
            meta_arg = "Engine::PropertyMeta{}" if not has_meta else format_meta_inline(p)
            if p.is_getter:
                get_lambda = f"[this]() {{ return this->{p.member}(); }}"
            else:
                get_lambda = "[this] { return " + p.member + "; }"
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
            if p.is_getter and has_setter_method:
                sn = setter_name if isinstance(setter_name, str) else ""
                getter_gs = {
                    "float": f"[this](float v) {{ this->{sn}(v); }}",
                    "double": f"[this](double v) {{ this->{sn}(v); }}",
                    "bool": f"[this](bool v) {{ this->{sn}(v); }}",
                    "int": f"[this](std::int32_t v) {{ this->{sn}(static_cast<int>(v)); }}",
                    "std::int32_t": f"[this](std::int32_t v) {{ this->{sn}(v); }}",
                    "std::int64_t": f"[this](std::int64_t v) {{ this->{sn}(v); }}",
                    "std::string": f"[this](std::string v) {{ this->{sn}(std::move(v)); }}",
                    "sf::Vector2f": f"[this](sf::Vector2f v) {{ this->{sn}(v); }}",
                    "sf::Vector3f": f"[this](sf::Vector3f v) {{ this->{sn}(v); }}",
                    "sf::Color": f"[this](sf::Color c) {{ this->{sn}(c); }}",
                }
                set_lambda = getter_gs[p.cpp_type]
            elif (
                isinstance(setter_method, str)
                and setter_method
                and not readonly
                and not p.is_getter
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
