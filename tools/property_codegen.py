#!/usr/bin/env python3
"""
Scan src/**/*.h for META_CLASS(), META_PROPERTY_BASE(), META_ENUM(), /// @property, /// @getter,
/// @setter, and /// @method tags; emit src/Codegen/<Stem>.generated.hpp and
/// src/Codegen/SceneEntityRegistry.generated.cpp (scene entity registration).
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
CACHE_VERSION = 21

PROPERTY_TAG_RE = re.compile(r"^\s*///\s*@property\s*(?:\((.*)\))?\s*$")
GETTER_TAG_RE = re.compile(r"^\s*///\s*@getter\s*(?:\((.*)\))?\s*$")
SETTER_TAG_RE = re.compile(r"^\s*///\s*@setter\s*(?:\((.*)\))?\s*$")
METHOD_TAG_RE = re.compile(r"^\s*///\s*@method\s*(?:\((.*)\))?\s*$")
VALUES_PROVIDER_TAG_RE = re.compile(
    r"^\s*///\s*@valuesProvider\s*\(\s*([A-Za-z_]\w*)\s*\)\s*$"
)
META_CLASS_RE = re.compile(r"\bMETA_CLASS\s*\(\s*\)")
META_PROPERTY_BASE_RE = re.compile(
    r"\bMETA_PROPERTY_BASE\s*\(\s*((?:[A-Za-z_]\w*\s*::\s*)*[A-Za-z_]\w*)\s*\)"
)
CLASS_HEAD_RE = re.compile(
    r"^\s*(?:template\s*<[^>{};]*>\s*)?(?:class|struct)\s+([A-Za-z_]\w*)\b"
)
NS_HEAD_RE = re.compile(r"^\s*namespace\s+([A-Za-z_]\w*)\s*(?:\{)?\s*$")
# Element / component types for std::vector, std::pair, std::optional, std::map, std::set (same set everywhere).
_VEC_ELT = (
    r"(float|double|bool|int|std::int32_t|std::int64_t|std::string|sf::Vector2f|sf::Vector2i|sf::Vector2u|"
    r"sf::Vector3f|sf::Color|sf::Angle|sf::IntRect|sf::FloatRect)"
)
_RECT_TYPES = frozenset({"sf::IntRect", "sf::FloatRect"})
_RECT_COMPONENT_VEC = {"sf::IntRect": "sf::Vector2i", "sf::FloatRect": "sf::Vector2f"}
_MOVE_BY_VALUE_TYPES = frozenset(
    {
        "std::string",
        "sf::Vector2f",
        "sf::Vector2i",
        "sf::Vector2u",
        "sf::Vector3f",
        "sf::Color",
        "sf::IntRect",
        "sf::FloatRect",
    }
)
FIELD_RE = re.compile(
    r"^\s*(?:inline\s+|static\s+|mutable\s+)*" + _VEC_ELT + r"\s+" + r"(\w+)\s*.*;\s*$"
)
REFWRAPPER_FIELD_RE = re.compile(
    r"^\s*(?:inline\s+|static\s+|mutable\s+)*RefWrapper\s*<\s*((?:[A-Za-z_]\w*\s*::\s*)*[A-Za-z_]\w*)\s*>\s+(\w+)\s*;\s*$"
)
VECTOR_FIELD_RE = re.compile(
    r"^\s*(?:inline\s+|static\s+|mutable\s+)*std::vector<\s*"
    + _VEC_ELT
    + r"\s*>\s+(\w+)\s*;\s*$"
)
PAIR_FIELD_RE = re.compile(
    r"^\s*(?:inline\s+|static\s+|mutable\s+)*std::pair\s*<\s*"
    + _VEC_ELT
    + r"\s*,\s*"
    + _VEC_ELT
    + r"\s*>\s+(\w+)\s*(?:\{[^}]*\}|=\s*[^;]+)?\s*;\s*$"
)
OPTIONAL_FIELD_RE = re.compile(
    r"^\s*(?:inline\s+|static\s+|mutable\s+)*std::optional<\s*"
    + _VEC_ELT
    + r"\s*>\s+(\w+)\s*(?:\{[^}]*\}|=\s*[^;]+)?\s*;\s*$"
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
    + _VEC_ELT
    + r"(?:\s*&)?\s+"
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
GETTER_OPTIONAL_VAL_RE = re.compile(
    r"^\s*(?:\[\[[^\]]*\]\]\s+)*(?!static\b)(?:virtual\s+)?"
    r"std::optional<\s*" + _VEC_ELT + r"\s*>\s+"
    r"(\w+)\s*\([^)]*\)\s*(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\s*;\s*$"
)
# void Method(Type param); — one parameter, known value type (v1).
SETTER_METHOD_RE = re.compile(
    r"^\s*(?:\[\[[^\]]*\]\]\s+)*(?!static\b)(?:virtual\s+)?void\s+"
    r"(\w+)\s*\(\s*(?:const\s+)?"
    + _VEC_ELT
    + r"(?:\s*&)?\s*\w*\s*\)\s*"
    r"(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\s*;\s*$"
)
SETTER_VECTOR_METHOD_RE = re.compile(
    r"^\s*(?:\[\[[^\]]*\]\]\s+)*(?!static\b)(?:virtual\s+)?void\s+"
    r"(\w+)\s*\(\s*(?:const\s+)?std::vector<\s*" + _VEC_ELT + r"\s*>\s*(?:const\s*)?(?:&)?\s*\w+\s*\)\s*"
    r"(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\s*;\s*$"
)
SETTER_OPTIONAL_METHOD_RE = re.compile(
    r"^\s*(?:\[\[[^\]]*\]\]\s+)*(?!static\b)(?:virtual\s+)?void\s+"
    r"(\w+)\s*\(\s*(?:const\s+)?std::optional<\s*" + _VEC_ELT + r"\s*>\s*(?:const\s*)?(?:&)?\s*\w+\s*\)\s*"
    r"(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\s*;\s*$"
)
# void Method(); — no parameters (v1).
VOID_METHOD_RE = re.compile(
    r"^\s*(?:\[\[[^\]]*\]\]\s+)*(?!static\b)(?:virtual\s+)?void\s+"
    r"(\w+)\s*\(\s*\)\s*"
    r"(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\s*;\s*$"
)
_QUALIFIED_ID_ENUM = r"[A-Za-z_]\w*(?:::[A-Za-z_]\w*)*"
META_ENUM_LINE_RE = re.compile(
    r"^\s*META_ENUM\s*\(\s*([A-Za-z_]\w*)\s*,\s*((?:[A-Za-z_]\w*)(?:\s*,\s*[A-Za-z_]\w*)*)\s*\)\s*;\s*$"
)
ENUM_PROP_FIELD_RE = re.compile(
    r"^\s*(?:inline\s+|static\s+|mutable\s+)*"
    + rf"({_QUALIFIED_ID_ENUM})"
    + r"\s+(\w+)\s*(?:=\s*[^;]+)?\s*;\s*$"
)
OBJECT_PROP_FIELD_RE = re.compile(
    r"^\s*(?:inline\s+|static\s+|mutable\s+)*"
    + rf"({_QUALIFIED_ID_ENUM})"
    + r"\s+(\w+)\s*(?:=\s*[^;]+)?\s*;\s*$"
)
GETTER_ENUM_METHOD_RE = re.compile(
    r"^\s*(?:\[\[[^\]]*\]\]\s+)*(?!static\b)(?:virtual\s+)?"
    r"(?:const\s+)?"
    + rf"({_QUALIFIED_ID_ENUM})(?:\s*&)?\s+"
    + r"(\w+)\s*\([^)]*\)\s*"
    r"(?:const\s*)?(?:noexcept\s*)?(?:override\s*)?(?:final\s*)?\s*;\s*$"
)
SETTER_ENUM_METHOD_RE = re.compile(
    r"^\s*(?:\[\[[^\]]*\]\]\s+)*(?!static\b)(?:virtual\s+)?void\s+"
    r"(\w+)\s*\(\s*(?:const\s+)?"
    + rf"({_QUALIFIED_ID_ENUM})(?:\s*&)?\s*\w+\s*\)\s*"
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
        "sf::Vector2i",
        "sf::Vector2u",
        "sf::Vector3f",
        "sf::Color",
        "sf::Angle",
        "sf::IntRect",
        "sf::FloatRect",
    }
)


def _is_rect_type(t: str) -> bool:
    return t in _RECT_TYPES


def _rect_vec_type(rect_t: str) -> str:
    return _RECT_COMPONENT_VEC[rect_t]


def _reject_rect_in_pair_types(ft: str, st: str, path: Path, line: int, col: int) -> None:
    if _is_rect_type(ft) or _is_rect_type(st):
        raise ParseError(
            "std::pair does not support sf::IntRect/sf::FloatRect in v1 "
            "(use a rect field, std::optional<sf::IntRect>, or @getter/@setter)",
            path,
            line,
            col,
        )


def _rect_grid_add_body(mem: str, rect_t: str, var_name: str, commit_stmt: str, tab: str) -> str:
    """Pick a rect not already in mem, then run commit_stmt (e.g. emplace/insert)."""
    if rect_t == "sf::IntRect":
        decl = f"sf::IntRect {var_name}" + "{{{{_ox, _oy}}, {{1, 1}}}};"
    elif rect_t == "sf::FloatRect":
        decl = (
            f"sf::FloatRect {var_name}"
            + "{{{{static_cast<float>(_ox), static_cast<float>(_oy)}}, {{1.f, 1.f}}}};"
        )
    else:
        raise ValueError(f"unexpected rect type `{rect_t}`")
    return (
        f"{tab}{{\n"
        f"{tab}\tbool _placed = false;\n"
        f"{tab}\tfor (int _ox = 0; _ox < 1000 && !_placed; ++_ox) {{\n"
        f"{tab}\t\tfor (int _oy = 0; _oy < 1000 && !_placed; ++_oy) {{\n"
        f"{tab}\t\t\t{decl}\n"
        f"{tab}\t\t\tif ({mem}.find({var_name}) == {mem}.end()) {{\n"
        f"{tab}\t\t\t\t{commit_stmt}\n"
        f"{tab}\t\t\t\t_placed = true;\n"
        f"{tab}\t\t\t}}\n"
        f"{tab}\t\t}}\n"
        f"{tab}\t}}\n"
        f"{tab}}}"
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
    is_pair: bool = False
    is_optional: bool = False
    is_object: bool = False
    is_ref_wrapper: bool = False
    ref_inner_type: str | None = None
    map_value_type: str | None = None
    assoc_container: str | None = None
    enum_enumerators: tuple[str, ...] | None = None


@dataclass
class MethodSpec:
    method: str
    line: int
    col: int
    attrs: dict[str, Any] = field(default_factory=dict)


PendingKind = Literal["property", "getter", "setter", "method"]


@dataclass
class PendingTag:
    kind: PendingKind
    line: int
    col: int
    args: str
    values_provider: str | None = None


PENDING_TAG_NAME: dict[PendingKind, str] = {
    "property": "@property",
    "getter": "@getter",
    "setter": "@setter",
    "method": "@method",
}

PENDING_TAGS_UNFINISHED = (
    "unfinished @property, @getter, @setter, or @method (a new tag started before the declaration line)"
)


@dataclass
class GetterDecl:
    cpp_type: str
    method: str
    line: int
    col: int
    attrs: dict[str, Any]
    is_vector: bool = False
    is_optional: bool = False
    enum_enumerators: tuple[str, ...] | None = None


@dataclass
class SetterDecl:
    cpp_type: str
    method: str
    line: int
    col: int
    attrs: dict[str, Any]
    is_vector: bool = False
    is_optional: bool = False
    enum_enumerators: tuple[str, ...] | None = None


def pair_tag_key(attrs: dict[str, Any]) -> str | None:
    for k in ("name", "label"):
        v = attrs.get(k)
        if isinstance(v, str) and v.strip():
            return v.strip()
    return None


def normalize_pair_key(s: str) -> str:
    return s.strip().casefold()


def implicit_accessor_core_suffix(method: str, role: Literal["get", "set"]) -> str | None:
    prefs = ("Get", "get") if role == "get" else ("Set", "set")
    for pref in prefs:
        if len(method) <= len(pref):
            continue
        if not method.startswith(pref):
            continue
        if not method[len(pref)].isupper():
            continue
        return method[len(pref) :]
    return None


def getter_pair_raw_key(g: GetterDecl) -> str | None:
    return pair_tag_key(g.attrs) or implicit_accessor_core_suffix(g.method, "get")


def setter_pair_raw_key(s: SetterDecl) -> str | None:
    return pair_tag_key(s.attrs) or implicit_accessor_core_suffix(s.method, "set")


def merge_getter_setter_decls(
    path: Path,
    getters: list[GetterDecl],
    setters: list[SetterDecl],
) -> list[PropSpec]:
    if not getters and not setters:
        return []

    by_key: dict[str, SetterDecl] = {}
    for s in setters:
        raw = setter_pair_raw_key(s)
        if raw is None:
            raise ParseError(
                '@setter requires name="..." or label="..." to pair with @getter, '
                "or use void SetXxx(...) with a matching GetXxx(...) (same Xxx after Get/Set)",
                path,
                s.line,
                s.col,
            )
        nk = normalize_pair_key(raw)
        if nk in by_key:
            raise ParseError(
                f'duplicate @setter for name="{raw}"',
                path,
                s.line,
                s.col,
            )
        by_key[nk] = s

    seen_gkeys: set[str] = set()
    out: list[PropSpec] = []

    for g in getters:
        graw = getter_pair_raw_key(g)
        if graw is not None:
            nk = normalize_pair_key(graw)
            if nk in seen_gkeys:
                raise ParseError(f'duplicate @getter for name="{graw}"', path, g.line, g.col)
            seen_gkeys.add(nk)

        merged = dict(g.attrs)
        inline = g.attrs.get("setter")
        inline_ok = isinstance(inline, str) and bool(inline.strip())

        setter_method: str | None = None
        if inline_ok:
            setter_method = inline.strip()
            if graw is not None and normalize_pair_key(graw) in by_key:
                raise ParseError(
                    f'@getter uses both setter= and @setter with name="{graw}"; use only one',
                    path,
                    g.line,
                    g.col,
                )
        elif graw is not None and normalize_pair_key(graw) in by_key:
            s = by_key.pop(normalize_pair_key(graw))
            if (
                s.cpp_type != g.cpp_type
                or s.is_vector != g.is_vector
                or s.is_optional != g.is_optional
            ):
                raise ParseError(
                    f'@getter / @setter pair "{graw}": type mismatch ({g.cpp_type} vs {s.cpp_type})',
                    path,
                    g.line,
                    g.col,
                )
            setter_method = s.method
            for k, v in s.attrs.items():
                if k not in merged:
                    merged[k] = v

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
                is_pair=False,
                is_optional=g.is_optional,
                map_value_type=None,
                assoc_container=None,
                enum_enumerators=g.enum_enumerators,
            )
        )

    if by_key:
        s = next(iter(by_key.values()))
        k = setter_pair_raw_key(s) or "?"
        raise ParseError(f'@setter name="{k}" has no matching @getter', path, s.line, s.col)

    return out


@dataclass
class ClassSpec:
    namespaces: tuple[str, ...]
    class_name: str
    props: list[PropSpec] = field(default_factory=list)
    methods: list[MethodSpec] = field(default_factory=list)
    property_base: str | None = None

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


def collect_inheritance_property_wrapper_ids(root: Path) -> frozenset[str]:
    """Property ids (snake_case class names) removed by flattened META_PROPERTY_BASE trees."""
    targets = collect_property_base_targets(root)
    wrapper_ids: set[str] = set()
    src = root / "src"
    for h in sorted(src.rglob("*.h"), key=lambda p: p.as_posix()):
        if "Codegen" in h.parts:
            continue
        classes, _ = parse_header(h)
        for c in classes:
            short = c.class_name
            if c.property_base or short in targets or c.qualified() in targets:
                wrapper_ids.add(pascal_to_snake(short))
    return frozenset(wrapper_ids)


def flatten_inheritance_wrappers_in_properties(properties: Any, wrapper_ids: frozenset[str]) -> bool:
    import xml.etree.ElementTree as ET

    if properties.tag != "Properties":
        return False
    new_children: list[ET.Element] = []
    changed = False
    for child in list(properties):
        if (
            child.tag == "Property"
            and child.get("kind") == "Object"
            and child.get("id") in wrapper_ids
        ):
            new_children.extend(list(child))
            changed = True
        else:
            new_children.append(child)
    if changed:
        properties[:] = new_children
    return changed


def migrate_scene_property_wrappers(scene_path: Path, wrapper_ids: frozenset[str]) -> int:
    import xml.etree.ElementTree as ET

    if not scene_path.is_file():
        return 0
    tree = ET.parse(scene_path)
    passes = 0
    while True:
        any_changed = False
        for properties in tree.getroot().iter("Properties"):
            if flatten_inheritance_wrappers_in_properties(properties, wrapper_ids):
                any_changed = True
        if not any_changed:
            break
        passes += 1
    if passes:
        ET.indent(tree, space="  ")
        tree.write(scene_path, encoding="utf-8", xml_declaration=True)
    return passes


def collect_property_base_targets(root: Path) -> set[str]:
    """Class names (short and qualified) referenced by META_PROPERTY_BASE(...)."""
    targets: set[str] = set()
    src = root / "src"
    for h in sorted(src.rglob("*.h"), key=lambda p: p.as_posix()):
        if "Codegen" in h.parts:
            continue
        classes, _ = parse_header(h)
        for c in classes:
            if not c.property_base:
                continue
            base = c.property_base
            targets.add(base)
            if "::" in base:
                targets.add(base.rsplit("::", 1)[-1])
    return targets


def class_uses_property_tree_wrapper(c: ClassSpec, property_base_targets: set[str]) -> bool:
    """True when BuildPropertyTree should wrap this class's properties in a pushObject section."""
    if c.property_base is not None:
        return False
    q = c.qualified()
    if q in property_base_targets or c.class_name in property_base_targets:
        return False
    return True


def member_to_field_id(member: str) -> str:
    return pascal_to_snake(member.lstrip("_"))


def _explicit_display_name(attrs: dict[str, Any]) -> str | None:
    for k in ("name", "label"):
        v = attrs.get(k)
        if isinstance(v, str) and v.strip():
            return v.strip()
    return None


def _split_identifier_words(raw: str) -> list[str]:
    s = raw.lstrip("_").strip()
    if not s:
        return []
    if "_" in s:
        return [p.lower() for p in s.split("_") if p]
    s1 = re.sub(r"(.)([A-Z][a-z]+)", r"\1 \2", s)
    s2 = re.sub(r"([a-z0-9])([A-Z])", r"\1 \2", s1)
    return [p.lower() for p in s2.split() if p]


def _strip_accessor_prefix(identifier: str, *, is_getter: bool) -> str:
    n = identifier.lstrip("_")
    suf = implicit_accessor_core_suffix(n, "get" if is_getter else "set")
    return suf if suf is not None else n


def _words_to_sentence_label(words: list[str]) -> str:
    if not words:
        return ""
    first, *rest = words
    head = (first[:1].upper() + first[1:].lower()) if first else ""
    tail = [w.lower() for w in rest]
    return " ".join([head, *tail]) if tail else head


def inferred_display_label(member: str, attrs: dict[str, Any], *, is_getter: bool) -> str:
    explicit = _explicit_display_name(attrs)
    if explicit is not None:
        return explicit
    core = _strip_accessor_prefix(member, is_getter=is_getter)
    words = _split_identifier_words(core)
    if not words:
        fid = member_to_field_id(member)
        return fid.replace("_", " ").title()
    return _words_to_sentence_label(words)


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


def class_enum_scope_parts(ns_stack: list[str], block_stack: list[dict[str, Any]]) -> list[str]:
    parts: list[str] = list(ns_stack)
    for b in block_stack:
        if b.get("kind") == "class" and isinstance(b.get("name"), str):
            parts.append(b["name"])
    return parts


def register_meta_enum(
    path: Path,
    line_no: int,
    enum_name: str,
    enumerators_csv: str,
    ns_stack: list[str],
    block_stack: list[dict[str, Any]],
    file_enums: dict[str, tuple[str, ...]],
) -> None:
    ids = [x.strip() for x in enumerators_csv.split(",") if x.strip()]
    if not ids:
        raise ParseError("META_ENUM requires at least one enumerator", path, line_no, 1)
    for eid in ids:
        if not re.fullmatch(r"[A-Za-z_]\w*", eid):
            raise ParseError(f"invalid enumerator `{eid}` in META_ENUM", path, line_no, 1)
    parts = class_enum_scope_parts(ns_stack, block_stack) + [enum_name]
    fq = "::".join(parts)
    if fq in file_enums:
        raise ParseError(f"duplicate META_ENUM for `{fq}`", path, line_no, 1)
    file_enums[fq] = tuple(ids)


def resolve_reflected_enum_type(
    raw_type: str,
    class_namespaces: tuple[str, ...],
    file_enums: dict[str, tuple[str, ...]],
) -> tuple[str, tuple[str, ...]] | None:
    t = re.sub(r"\s*::\s*", "::", raw_type.strip())
    if t in file_enums:
        return t, file_enums[t]
    if "::" not in t:
        candidate = "::".join((*class_namespaces, t))
        if candidate in file_enums:
            return candidate, file_enums[candidate]
    return None


def resolve_reflected_object_type(
    raw_type: str,
    class_namespaces: tuple[str, ...],
    reflected_types: set[str],
) -> str | None:
    t = re.sub(r"\s*::\s*", "::", raw_type.strip())
    if t in reflected_types:
        return t
    if "::" not in t:
        candidate = "::".join((*class_namespaces, t))
        if candidate in reflected_types:
            return candidate
    return None


def parse_header(path: Path) -> tuple[list[ClassSpec], list[str]]:
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()
    log: list[str] = []

    ns_stack: list[str] = []
    block_stack: list[dict[str, Any]] = []
    pending_class: str | None = None
    pending_ns: str | None = None
    finished: list[ClassSpec] = []
    pending: PendingTag | None = None
    file_enums: dict[str, tuple[str, ...]] = {}

    def close_class_block(b: dict[str, Any]) -> None:
        if b.get("property_base") and not b.get("meta"):
            raise ParseError(
                "META_PROPERTY_BASE(...) requires META_CLASS() in the same class",
                path,
                b.get("property_base_line", 1),
                1,
            )
        if not b.get("meta"):
            return
        props: list[PropSpec] = list(b.get("props", []))
        methods: list[MethodSpec] = list(b.get("methods", []))
        gdecls: list[GetterDecl] = list(b.get("getter_decls", []))
        sdecls: list[SetterDecl] = list(b.get("setter_decls", []))
        if gdecls or sdecls:
            props.extend(merge_getter_setter_decls(path, gdecls, sdecls))
        pb_raw = b.get("property_base")
        property_base: str | None = None
        if isinstance(pb_raw, str) and pb_raw.strip():
            property_base = re.sub(r"\s*::\s*", "::", pb_raw.strip())
        finished.append(
            ClassSpec(
                namespaces=tuple(b["namespaces"]),
                class_name=b["name"],
                props=props,
                methods=methods,
                property_base=property_base,
            )
        )

    for i, raw_line in enumerate(lines):
        line_no = i + 1
        line = strip_line_comment_keep_doc(raw_line)
        stripped = line.strip()

        m_values_provider = VALUES_PROVIDER_TAG_RE.match(raw_line)
        if m_values_provider:
            if pending is None:
                raise ParseError(
                    "@valuesProvider(...) must follow @property or @getter on a preceding line",
                    path,
                    line_no,
                    1,
                )
            if pending.kind not in ("property", "getter"):
                raise ParseError(
                    "@valuesProvider(...) applies only after @property or @getter",
                    path,
                    line_no,
                    1,
                )
            if pending.values_provider is not None:
                raise ParseError("duplicate @valuesProvider(...) for the same property", path, line_no, 1)
            pending.values_provider = m_values_provider.group(1)
            continue

        if pending is not None:
            if not stripped or stripped.startswith("///"):
                continue
            if re.match(r"^\s*(public|private|protected)\s*:\s*$", line):
                continue
            inner = find_innermost_class(block_stack)
            if inner is None or not inner.get("meta"):
                tag = PENDING_TAG_NAME[pending.kind]
                raise ParseError(
                    f"{tag} must appear inside a class marked with META_CLASS()",
                    path,
                    pending.line,
                    pending.col,
                )
            attrs = parse_attr_dict(pending.args, path, pending.line, pending.col)
            if pending.values_provider is not None:
                if "valuesProvider" in attrs:
                    raise ParseError(
                        "valuesProvider is specified both in the tag and in @valuesProvider(...)",
                        path,
                        line_no,
                        1,
                    )
                attrs["valuesProvider"] = pending.values_provider
            kind = pending.kind
            pline = pending.line
            pcol = pending.col
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
                elif m_opt := OPTIONAL_FIELD_RE.match(line):
                    et, member = m_opt.group(1), m_opt.group(2)
                    if et not in KNOWN_TYPES:
                        raise ParseError(
                            f"std::optional element must be a supported scalar type (tag at line {pline})",
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
                            is_set=False,
                            is_pair=False,
                            is_optional=True,
                            map_value_type=None,
                            assoc_container=None,
                        )
                    )
                elif m_pair := PAIR_FIELD_RE.match(line):
                    ft, st, member = m_pair.group(1), m_pair.group(2), m_pair.group(3)
                    if ft not in KNOWN_TYPES or st not in KNOWN_TYPES:
                        raise ParseError(
                            f"std::pair first/second must be supported scalar types (tag at line {pline})",
                            path,
                            line_no,
                            1,
                        )
                    _reject_rect_in_pair_types(ft, st, path, line_no, 1)
                    inner.setdefault("props", []).append(
                        PropSpec(
                            cpp_type=ft,
                            member=member,
                            line=pline,
                            col=pcol,
                            attrs=attrs,
                            is_getter=False,
                            is_vector=False,
                            is_bitset=False,
                            is_map=False,
                            is_set=False,
                            is_pair=True,
                            map_value_type=st,
                            assoc_container=None,
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
                    m_ref = REFWRAPPER_FIELD_RE.match(line)
                    if m_ref:
                        inner_t, member = m_ref.group(1), m_ref.group(2)
                        inner_t = re.sub(r"\s*::\s*", "::", inner_t.strip())
                        inner.setdefault("props", []).append(
                            PropSpec(
                                cpp_type=f"RefWrapper<{inner_t}>",
                                member=member,
                                line=pline,
                                col=pcol,
                                attrs=attrs,
                                is_getter=False,
                                is_vector=False,
                                is_bitset=False,
                                is_ref_wrapper=True,
                                ref_inner_type=inner_t,
                            )
                        )
                    else:
                        m_field = FIELD_RE.match(line)
                        if m_field:
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
                        else:
                            m_en = ENUM_PROP_FIELD_RE.match(line)
                            if m_en:
                                raw_t, member = m_en.group(1), m_en.group(2)
                                class_ns = tuple(inner["namespaces"])
                                resolved = resolve_reflected_enum_type(raw_t, class_ns, file_enums)
                                if resolved is None:
                                    raise ParseError(
                                        f"expected field with supported type after @property (unknown type `{raw_t}`, "
                                        f"tag at line {pline})",
                                        path,
                                        line_no,
                                        1,
                                    )
                                canon, en = resolved
                                inner.setdefault("props", []).append(
                                    PropSpec(
                                        cpp_type=canon,
                                        member=member,
                                        line=pline,
                                        col=pcol,
                                        attrs=attrs,
                                        is_getter=False,
                                        is_vector=False,
                                        is_bitset=False,
                                        enum_enumerators=en,
                                    )
                                )
                            else:
                                m_obj = OBJECT_PROP_FIELD_RE.match(line)
                                if m_obj:
                                    cpp_type, member = m_obj.group(1), m_obj.group(2)
                                    inner.setdefault("props", []).append(
                                        PropSpec(
                                            cpp_type=re.sub(r"\s*::\s*", "::", cpp_type.strip()),
                                            member=member,
                                            line=pline,
                                            col=pcol,
                                            attrs=attrs,
                                            is_getter=False,
                                            is_vector=False,
                                            is_bitset=False,
                                            is_object=True,
                                        )
                                    )
                                else:
                                    raise ParseError(
                                        f"expected field with supported type after @property (tag at line {pline})",
                                        path,
                                        line_no,
                                        1,
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
                elif m_opt := GETTER_OPTIONAL_VAL_RE.match(line):
                    cpp_type, method = m_opt.group(1), m_opt.group(2)
                    inner.setdefault("getter_decls", []).append(
                        GetterDecl(
                            cpp_type=cpp_type,
                            method=method,
                            line=pline,
                            col=pcol,
                            attrs=attrs,
                            is_vector=False,
                            is_optional=True,
                        )
                    )
                else:
                    m_getter_enum = GETTER_ENUM_METHOD_RE.match(line)
                    resolved_ge: tuple[str, tuple[str, ...]] | None = None
                    if m_getter_enum:
                        raw_t, method = m_getter_enum.group(1), m_getter_enum.group(2)
                        class_ns = tuple(inner["namespaces"])
                        resolved_ge = resolve_reflected_enum_type(raw_t, class_ns, file_enums)
                    if resolved_ge is not None:
                        canon, en = resolved_ge
                        inner.setdefault("getter_decls", []).append(
                            GetterDecl(
                                cpp_type=canon,
                                method=method,
                                line=pline,
                                col=pcol,
                                attrs=attrs,
                                is_vector=False,
                                enum_enumerators=en,
                            )
                        )
                    else:
                        raise ParseError(
                            f"expected instance method with supported return type after @getter (tag at line {pline})",
                            path,
                            line_no,
                            1,
                        )
            elif kind == "setter":
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
                elif m_setter_opt := SETTER_OPTIONAL_METHOD_RE.match(line):
                    smethod, stype = m_setter_opt.group(1), m_setter_opt.group(2)
                    inner.setdefault("setter_decls", []).append(
                        SetterDecl(
                            cpp_type=stype,
                            method=smethod,
                            line=pline,
                            col=pcol,
                            attrs=attrs,
                            is_vector=False,
                            is_optional=True,
                        )
                    )
                else:
                    m_setter_enum = SETTER_ENUM_METHOD_RE.match(line)
                    resolved_se: tuple[str, tuple[str, ...]] | None = None
                    if m_setter_enum:
                        smethod, raw_t = m_setter_enum.group(1), m_setter_enum.group(2)
                        class_ns = tuple(inner["namespaces"])
                        resolved_se = resolve_reflected_enum_type(raw_t, class_ns, file_enums)
                    if resolved_se is not None:
                        canon, en = resolved_se
                        inner.setdefault("setter_decls", []).append(
                            SetterDecl(
                                cpp_type=canon,
                                method=smethod,
                                line=pline,
                                col=pcol,
                                attrs=attrs,
                                is_vector=False,
                                enum_enumerators=en,
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
            else:
                if re.search(r"\bstatic\b", line):
                    raise ParseError(
                        f"@method does not support static methods (tag at line {pline})",
                        path,
                        line_no,
                        1,
                    )
                m_void = VOID_METHOD_RE.match(line)
                if not m_void:
                    raise ParseError(
                        f"expected void Method() after @method (tag at line {pline})",
                        path,
                        line_no,
                        1,
                    )
                inner.setdefault("methods", []).append(
                    MethodSpec(
                        method=m_void.group(1),
                        line=pline,
                        col=pcol,
                        attrs=attrs,
                    )
                )
            pending = None
            continue

        me = META_ENUM_LINE_RE.match(line)
        if me and not re.search(r"^\s*#\s*define\s+META_ENUM\b", raw_line):
            register_meta_enum(path, line_no, me.group(1), me.group(2), ns_stack, block_stack, file_enums)
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
                    PENDING_TAGS_UNFINISHED,
                    path,
                    line_no,
                    1,
                )
            args = m_prop.group(1) or ""
            try:
                col = raw_line.index("@property") + 1
            except ValueError:
                col = 1
            pending = PendingTag("property", line_no, col, args)
            continue

        m_getter_tag = GETTER_TAG_RE.match(raw_line)
        if m_getter_tag:
            if pending is not None:
                raise ParseError(
                    PENDING_TAGS_UNFINISHED,
                    path,
                    line_no,
                    1,
                )
            args = m_getter_tag.group(1) or ""
            try:
                col = raw_line.index("@getter") + 1
            except ValueError:
                col = 1
            pending = PendingTag("getter", line_no, col, args)
            continue

        m_setter_tag = SETTER_TAG_RE.match(raw_line)
        if m_setter_tag:
            if pending is not None:
                raise ParseError(
                    PENDING_TAGS_UNFINISHED,
                    path,
                    line_no,
                    1,
                )
            args = m_setter_tag.group(1) or ""
            try:
                col = raw_line.index("@setter") + 1
            except ValueError:
                col = 1
            pending = PendingTag("setter", line_no, col, args)
            continue

        m_method_tag = METHOD_TAG_RE.match(raw_line)
        if m_method_tag:
            if pending is not None:
                raise ParseError(
                    PENDING_TAGS_UNFINISHED,
                    path,
                    line_no,
                    1,
                )
            args = m_method_tag.group(1) or ""
            try:
                col = raw_line.index("@method") + 1
            except ValueError:
                col = 1
            pending = PendingTag("method", line_no, col, args)
            continue

        if META_CLASS_RE.search(line) and not re.search(r"^\s*#\s*define\s+META_CLASS\b", line):
            inner = find_innermost_class(block_stack)
            if inner is None:
                raise ParseError("META_CLASS() must appear inside a class definition", path, line_no, 1)
            inner["meta"] = True
            continue

        m_meta_base = META_PROPERTY_BASE_RE.search(line)
        if m_meta_base and not re.search(r"^\s*#\s*define\s+META_PROPERTY_BASE\b", line):
            inner = find_innermost_class(block_stack)
            if inner is None:
                raise ParseError(
                    "META_PROPERTY_BASE(...) must appear inside a class definition",
                    path,
                    line_no,
                    1,
                )
            if inner.get("property_base") is not None:
                raise ParseError(
                    "duplicate META_PROPERTY_BASE(...) in the same class",
                    path,
                    line_no,
                    1,
                )
            inner["property_base"] = m_meta_base.group(1)
            inner["property_base_line"] = line_no
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
                        "methods": [],
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
                    "methods": [],
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
                            "methods": [],
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
        tag = PENDING_TAG_NAME[pending.kind]
        raise ParseError(f"unfinished {tag} (no declaration line before EOF)", path, pending.line, pending.col)

    for spec in finished:
        extra = f", property_base={spec.property_base}" if spec.property_base else ""
        log.append(
            f"  class {spec.qualified()} : {len(spec.props)} properties, {len(spec.methods)} methods{extra}"
        )

    return finished, log


def default_label(p: PropSpec) -> str:
    return inferred_display_label(p.member, p.attrs, is_getter=p.is_getter)


def method_menu_label(m: MethodSpec) -> str:
    return inferred_display_label(m.method, m.attrs, is_getter=False)


def validate_values_provider_tag(p: PropSpec, path: Path) -> None:
    if p.enum_enumerators is not None:
        if p.attrs.get("valuesProvider") is not None:
            raise ParseError(
                "valuesProvider / @valuesProvider is not supported for META_ENUM properties",
                path,
                p.line,
                p.col,
            )
        return
    vp = p.attrs.get("valuesProvider")
    if vp is None:
        return
    if not isinstance(vp, str) or not vp.strip():
        raise ParseError("valuesProvider must be a non-empty identifier", path, p.line, p.col)
    if p.is_vector or p.is_bitset or p.is_map or p.is_set or p.is_pair or p.is_optional:
        raise ParseError(
            "@valuesProvider is not supported for container or bitset properties",
            path,
            p.line,
            p.col,
        )
    allowed_types = {"std::string", "int", "std::int32_t", "std::int64_t", "float", "double"}
    if p.cpp_type not in allowed_types:
        raise ParseError(
            f"@valuesProvider is not supported for type `{p.cpp_type}`",
            path,
            p.line,
            p.col,
        )


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
    vp = a.get("valuesProvider")
    if isinstance(vp, str) and vp.strip():
        fn = vp.strip()
        if p.cpp_type == "std::string":
            parts.append(
                "_m.valuesProviderStdString = []() -> std::vector<std::string> { "
                f"return Editor::ValuesProviderDetail::ToVector(Editor::ValuesProviders::{fn}()); "
                "};"
            )
        elif p.cpp_type in ("int", "std::int32_t"):
            parts.append(
                "_m.valuesProviderInt32 = []() -> std::vector<std::int32_t> { "
                f"auto _vpv = Editor::ValuesProviderDetail::ToVector(Editor::ValuesProviders::{fn}()); "
                "return std::vector<std::int32_t>{_vpv.begin(), _vpv.end()}; };"
            )
        elif p.cpp_type == "std::int64_t":
            parts.append(
                "_m.valuesProviderInt64 = []() -> std::vector<std::int64_t> { "
                f"auto _vpv = Editor::ValuesProviderDetail::ToVector(Editor::ValuesProviders::{fn}()); "
                "return std::vector<std::int64_t>{_vpv.begin(), _vpv.end()}; };"
            )
        elif p.cpp_type == "float":
            parts.append(
                "_m.valuesProviderFloat = []() -> std::vector<float> { "
                f"return Editor::ValuesProviderDetail::ToVector(Editor::ValuesProviders::{fn}()); "
                "};"
            )
        elif p.cpp_type == "double":
            parts.append(
                "_m.valuesProviderDouble = []() -> std::vector<double> { "
                f"return Editor::ValuesProviderDetail::ToVector(Editor::ValuesProviders::{fn}()); "
                "};"
            )
    inner = " ".join(parts)
    return f"[&]() -> Engine::PropertyMeta {{ {inner} return _m; }}()"


def emit_bitset_property(
    out: list[str],
    p: PropSpec,
    meta_arg: str,
    readonly: bool,
) -> None:
    fid = member_to_field_id(p.member)
    label_esc = cpp_escape_string(default_label(p))
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
        "sf::Vector2i": "sf::Vector2i{}",
        "sf::Vector2u": "sf::Vector2u{}",
        "sf::Vector3f": "sf::Vector3f{}",
        "sf::Color": "sf::Color{}",
        "sf::Angle": "sf::Angle{}",
        "sf::IntRect": "sf::IntRect{}",
        "sf::FloatRect": "sf::FloatRect{}",
    }[t]


def _int32_param_cpp(t: str) -> str:
    return "std::int32_t" if t == "int" else t


ScalarSetterMode = Literal["readonly", "assign", "call_setter", "vector_cow"]
MovePolicy = Literal["none", "string_only", "move_by_value"]


def _scalar_readonly_param_type(t: str) -> str:
    if t == "sf::Angle":
        return "float"
    if t in ("int", "std::int32_t"):
        return "std::int32_t"
    return t


def _scalar_set_param_type(t: str) -> str:
    if t == "sf::Angle":
        return "float"
    if t in ("int", "std::int32_t"):
        return "std::int32_t"
    return t


def _scalar_set_param_name(t: str) -> str:
    return "c" if t == "sf::Color" else "v"


def _scalar_assign_rhs(t: str, var: str, *, move_policy: MovePolicy) -> str:
    if t == "int":
        return f"static_cast<int>({var})"
    if t == "sf::Angle":
        return f"sf::degrees({var})"
    if t == "std::string":
        return f"std::move({var})"
    if move_policy == "move_by_value" and t in _MOVE_BY_VALUE_TYPES:
        return f"std::move({var})"
    return var


def _scalar_get_lambda(
    t: str,
    *,
    capture: str,
    read_expr: str,
    call_syntax: bool | None = None,
    prefix: str = "",
) -> str:
    if call_syntax is None:
        call_syntax = t in ("int", "sf::Angle")
    opener = f"{capture}() {{" if call_syntax else f"{capture} {{"
    if t == "int":
        return f"{opener} {prefix}return static_cast<std::int32_t>({read_expr}); }}"
    if t == "sf::Angle":
        return f"{opener} {prefix}return {read_expr}.asDegrees(); }}"
    return f"{opener} {prefix}return {read_expr}; }}"


def _scalar_set_lambda(
    t: str,
    *,
    mode: ScalarSetterMode,
    capture: str,
    move_policy: MovePolicy = "none",
    assign_lhs: str = "",
    assign_prefix: str = "",
    setter_name: str = "",
    vec_access: str = "",
    vec_index: str = "",
    param_name: str | None = None,
) -> str:
    if mode == "readonly":
        pk = _scalar_readonly_param_type(t)
        return f"{capture}({pk}) {{}}"
    pt = _scalar_set_param_type(t)
    pn = param_name if param_name is not None else _scalar_set_param_name(t)
    if mode == "assign":
        rhs = _scalar_assign_rhs(t, pn, move_policy=move_policy)
        return f"{capture}({pt} {pn}) {{ {assign_prefix}{assign_lhs} = {rhs}; }}"
    if mode == "call_setter":
        arg = _scalar_assign_rhs(t, pn, move_policy=move_policy)
        return f"{capture}({pt} {pn}) {{ this->{setter_name}({arg}); }}"
    if mode == "vector_cow":
        if t == "int":
            elem = f"static_cast<int>({pn})"
        elif t == "sf::Angle":
            elem = f"sf::degrees({pn})"
        elif t == "std::string":
            elem = f"std::move({pn})"
        else:
            elem = pn
        return (
            f"{capture}({pt} {pn}) {{ auto _pv = {vec_access}; "
            f"_pv[{vec_index}] = {elem}; this->{setter_name}(std::move(_pv)); }}"
        )
    raise ValueError(f"unknown ScalarSetterMode {mode!r}")


def _rect_component_get(acc: str, component: str) -> str:
    return f"[this]() {{ return {acc}.{component}; }}"


def _rect_component_set_field(acc: str, component: str, vec_t: str, readonly: bool) -> str:
    if readonly:
        return f"[this]({vec_t}) {{}}"
    if vec_t in _MOVE_BY_VALUE_TYPES:
        return f"[this]({vec_t} v) {{ {acc}.{component} = std::move(v); }}"
    return f"[this]({vec_t} v) {{ {acc}.{component} = v; }}"


def _rect_component_set_getter(
    get_expr: str, setter_name: str, component: str, rect_t: str, vec_t: str, readonly: bool
) -> str:
    if readonly:
        return f"[this]({vec_t}) {{}}"
    return (
        f"[this]({vec_t} v) {{\n"
        f"\t\tauto _r = {get_expr};\n"
        f"\t\t_r.{component} = v;\n"
        f"\t\tthis->{setter_name}(std::move(_r));\n"
        f"\t}}"
    )


def _rect_component_set_optional_field(
    storage: str, component: str, rect_t: str, vec_t: str, readonly: bool
) -> str:
    if readonly:
        return f"[this]({vec_t}) {{}}"
    default = _default_cpp_value(rect_t)
    return (
        f"[this]({vec_t} v) {{\n"
        f"\t\tif (!{storage}) {storage} = {default};\n"
        f"\t\t{storage}->{component} = v;\n"
        f"\t}}"
    )


def _rect_component_set_optional_getter(
    get_expr: str, setter_name: str, component: str, rect_t: str, vec_t: str, readonly: bool
) -> str:
    if readonly:
        return f"[this]({vec_t}) {{}}"
    default = _default_cpp_value(rect_t)
    return (
        f"[this]({vec_t} v) {{\n"
        f"\t\tauto _opt = {get_expr};\n"
        f"\t\tif (!_opt) _opt = {default};\n"
        f"\t\t_opt->{component} = v;\n"
        f"\t\tthis->{setter_name}(std::move(_opt));\n"
        f"\t}}"
    )


def _rect_component_get_optional(storage: str, component: str, rect_t: str) -> str:
    default = _default_cpp_value(rect_t)
    return (
        f"[this]() {{ const auto& _opt = {storage}; "
        f"return _opt ? _opt->{component} : {default}.{component}; }}"
    )


def _emit_rect_children(
    out: list[str],
    rect_t: str,
    path: Path,
    line: int,
    col: int,
    *,
    get_pos: str,
    set_pos: str,
    get_size: str,
    set_size: str,
    indent: str = "\t\t",
) -> None:
    vec_t = _rect_vec_type(rect_t)
    leaf_meta = "Engine::PropertyMeta{}"
    for nid, label, g, s in (
        ("position", "Position", get_pos, set_pos),
        ("size", "Size", get_size, set_size),
    ):
        _emit_scalar_leaf(
            out,
            vec_t,
            nid,
            label,
            g,
            s,
            leaf_meta,
            path,
            line,
            col,
            indent=indent,
        )


def emit_rect_property(
    out: list[str],
    p: PropSpec,
    path: Path,
    meta_arg: str,
    readonly: bool,
    setter_name: str = "",
) -> None:
    rect_t = p.cpp_type
    vec_t = _rect_vec_type(rect_t)
    fid = member_to_field_id(p.member)
    label_esc = cpp_escape_string(default_label(p))
    sn = setter_name.strip() if isinstance(setter_name, str) else ""

    out.append(f'\tb.pushObject("{fid}", "{label_esc}", {meta_arg});')
    if p.is_getter:
        if not readonly and not sn:
            raise ParseError(
                "writable @getter returning sf::Rect requires a paired @setter (or setter= on @getter)",
                path,
                p.line,
                p.col,
            )
        get_expr = f"this->{p.member}()"
        get_pos = _rect_component_get(get_expr, "position")
        get_size = _rect_component_get(get_expr, "size")
        set_pos = _rect_component_set_getter(get_expr, sn, "position", rect_t, vec_t, readonly)
        set_size = _rect_component_set_getter(get_expr, sn, "size", rect_t, vec_t, readonly)
    else:
        mem = p.member
        get_pos = _rect_component_get(mem, "position")
        get_size = _rect_component_get(mem, "size")
        set_pos = _rect_component_set_field(mem, "position", vec_t, readonly)
        set_size = _rect_component_set_field(mem, "size", vec_t, readonly)
    _emit_rect_children(
        out,
        rect_t,
        path,
        p.line,
        p.col,
        get_pos=get_pos,
        set_pos=set_pos,
        get_size=get_size,
        set_size=set_size,
    )
    out.append("\tb.pop();")


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
    *,
    indent: str = "\t\t",
) -> None:
    if _is_rect_type(t):
        raise ParseError(
            f"scalar leaf cannot emit whole `{t}` (use rect component helpers)",
            path,
            line,
            col,
        )
    if t == "float":
        out.append(f'{indent}b.addFloat("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "double":
        out.append(f'{indent}b.addDouble("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "bool":
        out.append(f'{indent}b.addBool("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t in ("int", "std::int32_t"):
        out.append(f'{indent}b.addInt32("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "std::int64_t":
        out.append(f'{indent}b.addInt64("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "std::string":
        out.append(f'{indent}b.addString("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "sf::Vector2f":
        out.append(f'{indent}b.addVec2f("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "sf::Vector2i":
        out.append(f'{indent}b.addVec2i("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "sf::Vector2u":
        out.append(f'{indent}b.addVec2u("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "sf::Vector3f":
        out.append(f'{indent}b.addVec3f("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "sf::Color":
        out.append(f'{indent}b.addColor("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    elif t == "sf::Angle":
        out.append(f'{indent}b.addFloat("{nid}", "{label_esc}", {get_l}, {set_l}, {meta});')
    else:
        raise ParseError(f"unsupported scalar type `{t}`", path, line, col)


def _map_key_get(mem: str, idx: str, kt: str) -> str:
    adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
    return _scalar_get_lambda(
        kt,
        capture=f"[this, {idx}]",
        read_expr="_it->first",
        call_syntax=True,
        prefix=adv,
    )


def _map_key_set(mem: str, idx: str, kt: str, readonly: bool) -> str:
    if readonly:
        return _scalar_set_lambda(kt, mode="readonly", capture=f"[this, {idx}]")
    adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
    mv = "auto _mapped = std::move(_it->second); " + f"{mem}.erase(_it); "
    if kt == "sf::Angle":
        ins = f"{mem}.insert_or_assign(sf::degrees(newKey), std::move(_mapped))"
        return f"[this, {idx}](float newKey) {{ {adv}{mv}{ins}; }}"
    pk = _int32_param_cpp(kt)
    if kt == "int":
        ins = f"{mem}.insert_or_assign(static_cast<int>(newKey), std::move(_mapped))"
    elif kt == "std::string":
        ins = f"{mem}.insert_or_assign(std::move(newKey), std::move(_mapped))"
    elif kt in _MOVE_BY_VALUE_TYPES:
        ins = f"{mem}.insert_or_assign(std::move(newKey), std::move(_mapped))"
    else:
        ins = f"{mem}.insert_or_assign(newKey, std::move(_mapped))"
    return f"[this, {idx}]({pk} newKey) {{ {adv}{mv}{ins}; }}"


def _map_val_get(mem: str, idx: str, vt: str) -> str:
    adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
    return _scalar_get_lambda(
        vt,
        capture=f"[this, {idx}]",
        read_expr="_it->second",
        call_syntax=True,
        prefix=adv,
    )


def _map_val_set(mem: str, idx: str, vt: str, readonly: bool) -> str:
    capture = f"[this, {idx}]"
    if readonly:
        return _scalar_set_lambda(vt, mode="readonly", capture=capture)
    adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
    return _scalar_set_lambda(
        vt,
        mode="assign",
        capture=capture,
        assign_lhs="_it->second",
        assign_prefix=adv,
        move_policy="move_by_value",
        param_name="newVal",
    )


def _cpp_map_emplace(mem: str, kt: str, key_var: str, dv: str) -> str:
    if kt in _MOVE_BY_VALUE_TYPES:
        return f"{mem}.emplace(std::move({key_var}), {dv})"
    return f"{mem}.emplace({key_var}, {dv})"


def _cpp_set_insert(mem: str, et: str, val_var: str) -> str:
    if et in _MOVE_BY_VALUE_TYPES:
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
    if kt == "sf::Vector2i":
        return (
            f"{tab}{{\n"
            f"{tab}\tbool _placed = false;\n"
            f"{tab}\tfor (int _ox = 0; _ox < 100000 && !_placed; ++_ox) {{\n"
            f"{tab}\t\tsf::Vector2i _nk{{_ox, 0}};\n"
            f"{tab}\t\tif ({mem}.find(_nk) == {mem}.end()) {{\n"
            f"{tab}\t\t\t{_cpp_map_emplace(mem, kt, '_nk', dv)};\n"
            f"{tab}\t\t\t_placed = true;\n"
            f"{tab}\t\t}}\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    if kt == "sf::Vector2u":
        return (
            f"{tab}{{\n"
            f"{tab}\tbool _placed = false;\n"
            f"{tab}\tfor (int _ox = 0; _ox < 100000 && !_placed; ++_ox) {{\n"
            f"{tab}\t\tsf::Vector2u _nk{{static_cast<unsigned int>(_ox), 0u}};\n"
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
    if kt == "sf::Angle":
        return (
            f"{tab}{{\n"
            f"{tab}\tfloat _d = 0.f;\n"
            f"{tab}\tfor (int _g = 0; _g < 1000000 && {mem}.find(sf::degrees(_d)) != {mem}.end(); ++_g) {{\n"
            f"{tab}\t\t_d += 1.f;\n"
            f"{tab}\t}}\n"
            f"{tab}\tif ({mem}.find(sf::degrees(_d)) == {mem}.end()) {{\n"
            f"{tab}\t\t{_cpp_map_emplace(mem, kt, 'sf::degrees(_d)', dv)};\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    if _is_rect_type(kt):
        commit = f"{_cpp_map_emplace(mem, kt, '_nk', dv)};"
        return _rect_grid_add_body(mem, kt, "_nk", commit, tab)
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
    if et == "sf::Vector2i":
        return (
            f"{tab}{{\n"
            f"{tab}\tbool _placed = false;\n"
            f"{tab}\tfor (int _ox = 0; _ox < 100000 && !_placed; ++_ox) {{\n"
            f"{tab}\t\tsf::Vector2i _nv{{_ox, 0}};\n"
            f"{tab}\t\tif ({mem}.find(_nv) == {mem}.end()) {{\n"
            f"{tab}\t\t\t{_cpp_set_insert(mem, et, '_nv')};\n"
            f"{tab}\t\t\t_placed = true;\n"
            f"{tab}\t\t}}\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    if et == "sf::Vector2u":
        return (
            f"{tab}{{\n"
            f"{tab}\tbool _placed = false;\n"
            f"{tab}\tfor (int _ox = 0; _ox < 100000 && !_placed; ++_ox) {{\n"
            f"{tab}\t\tsf::Vector2u _nv{{static_cast<unsigned int>(_ox), 0u}};\n"
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
    if et == "sf::Angle":
        return (
            f"{tab}{{\n"
            f"{tab}\tfloat _d = 0.f;\n"
            f"{tab}\tfor (int _g = 0; _g < 1000000 && {mem}.find(sf::degrees(_d)) != {mem}.end(); ++_g) {{\n"
            f"{tab}\t\t_d += 1.f;\n"
            f"{tab}\t}}\n"
            f"{tab}\tif ({mem}.find(sf::degrees(_d)) == {mem}.end()) {{\n"
            f"{tab}\t\t{_cpp_set_insert(mem, et, 'sf::degrees(_d)')};\n"
            f"{tab}\t}}\n"
            f"{tab}}}"
        )
    if _is_rect_type(et):
        commit = f"{_cpp_set_insert(mem, et, '_nv')};"
        return _rect_grid_add_body(mem, et, "_nv", commit, tab)
    raise ParseError(f"set addPair: unsupported element type `{et}`", path, line, col)


def _map_rect_component_get(mem: str, idx: str, side: str, component: str) -> str:
    adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
    field = "first" if side == "key" else "second"
    return f"[this, {idx}]() {{ {adv}return _it->{field}.{component}; }}"


def _map_rect_component_set_key(mem: str, idx: str, component: str, vec_t: str, readonly: bool) -> str:
    if readonly:
        return f"[this, {idx}]({vec_t}) {{}}"
    adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
    return (
        f"[this, {idx}]({vec_t} v) {{\n"
        f"\t\t{adv}\n"
        f"\t\tauto _mapped = std::move(_it->second);\n"
        f"\t\tauto _nk = _it->first;\n"
        f"\t\t{mem}.erase(_it);\n"
        f"\t\t_nk.{component} = v;\n"
        f"\t\t{mem}.emplace(std::move(_nk), std::move(_mapped));\n"
        f"\t}}"
    )


def _map_rect_component_set_val(mem: str, idx: str, component: str, vec_t: str, readonly: bool) -> str:
    if readonly:
        return f"[this, {idx}]({vec_t}) {{}}"
    adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
    return f"[this, {idx}]({vec_t} v) {{ {adv}_it->second.{component} = v; }}"


def _set_rect_component_get(mem: str, idx: str, component: str) -> str:
    adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
    return f"[this, {idx}]() {{ {adv}return _it->{component}; }}"


def _set_rect_component_set(mem: str, idx: str, component: str, vec_t: str, readonly: bool) -> str:
    if readonly:
        return f"[this, {idx}]({vec_t}) {{}}"
    adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
    return (
        f"[this, {idx}]({vec_t} v) {{\n"
        f"\t\t{adv}\n"
        f"\t\tauto _nv = *_it;\n"
        f"\t\t{mem}.erase(_it);\n"
        f"\t\t_nv.{component} = v;\n"
        f"\t\t{mem}.insert(std::move(_nv));\n"
        f"\t}}"
    )


def _emit_map_rect_side(
    out: list[str],
    mem: str,
    idx: str,
    rect_t: str,
    side: str,
    label: str,
    readonly: bool,
    path: Path,
    line: int,
    col: int,
) -> None:
    vec_t = _rect_vec_type(rect_t)
    leaf_meta = "Engine::PropertyMeta{}"
    out.append(f'\t\tb.pushObject("{side}", "{label}", {leaf_meta});')
    if side == "key":
        get_pos = _map_rect_component_get(mem, idx, "key", "position")
        get_size = _map_rect_component_get(mem, idx, "key", "size")
        set_pos = _map_rect_component_set_key(mem, idx, "position", vec_t, readonly)
        set_size = _map_rect_component_set_key(mem, idx, "size", vec_t, readonly)
    else:
        get_pos = _map_rect_component_get(mem, idx, "value", "position")
        get_size = _map_rect_component_get(mem, idx, "value", "size")
        set_pos = _map_rect_component_set_val(mem, idx, "position", vec_t, readonly)
        set_size = _map_rect_component_set_val(mem, idx, "size", vec_t, readonly)
    _emit_rect_children(
        out,
        rect_t,
        path,
        line,
        col,
        get_pos=get_pos,
        set_pos=set_pos,
        get_size=get_size,
        set_size=set_size,
        indent="\t\t\t",
    )
    out.append("\t\tb.pop();")


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
    label_esc = cpp_escape_string(default_label(p))
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
    if _is_rect_type(kt):
        _emit_map_rect_side(out, mem, idx, kt, "key", "Key", readonly, path, p.line, p.col)
    else:
        gk, sk = _map_key_get(mem, idx, kt), _map_key_set(mem, idx, kt, readonly)
        _emit_scalar_leaf(out, kt, "key", "Key", gk, sk, leaf_meta, path, p.line, p.col)
    if _is_rect_type(vt):
        _emit_map_rect_side(out, mem, idx, vt, "value", "Value", readonly, path, p.line, p.col)
    else:
        gv, sv = _map_val_get(mem, idx, vt), _map_val_set(mem, idx, vt, readonly)
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
    label_esc = cpp_escape_string(default_label(p))
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
    if _is_rect_type(et):
        vec_t = _rect_vec_type(et)
        get_pos = _set_rect_component_get(mem, idx, "position")
        get_size = _set_rect_component_get(mem, idx, "size")
        set_pos = _set_rect_component_set(mem, idx, "position", vec_t, readonly)
        set_size = _set_rect_component_set(mem, idx, "size", vec_t, readonly)
        _emit_rect_children(
            out,
            et,
            path,
            p.line,
            p.col,
            get_pos=get_pos,
            set_pos=set_pos,
            get_size=get_size,
            set_size=set_size,
        )
    else:
        adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); "
        g = _scalar_get_lambda(
            et,
            capture=f"[this, {idx}]",
            read_expr="*_it",
            call_syntax=True,
            prefix=adv,
        )
        if readonly:
            s = _scalar_set_lambda(et, mode="readonly", capture=f"[this, {idx}]")
        else:
            adv = f"auto _it = {mem}.begin(); std::advance(_it, {idx}); {mem}.erase(_it); "
            if et == "int":
                s = f"[this, {idx}](std::int32_t v) {{ {adv}{mem}.insert(static_cast<int>(v)); }}"
            elif et == "sf::Angle":
                s = f"[this, {idx}](float v) {{ {adv}{mem}.insert(sf::degrees(v)); }}"
            elif et == "std::string":
                s = f"[this, {idx}](std::string v) {{ {adv}{mem}.insert(std::move(v)); }}"
            elif et in _MOVE_BY_VALUE_TYPES:
                s = f"[this, {idx}]({et} v) {{ {adv}{mem}.insert(std::move(v)); }}"
            else:
                s = f"[this, {idx}]({et} v) {{ {adv}{mem}.insert(v); }}"
        _emit_scalar_leaf(out, et, "v", "", g, s, leaf_meta, path, p.line, p.col)
    out.append("\t\tb.pop();")
    out.append("\t}")
    out.append("\tb.endAssociative();")


def emit_std_pair_property(
    out: list[str],
    p: PropSpec,
    path: Path,
    meta_arg: str,
    readonly: bool,
) -> None:
    st = p.map_value_type
    if st is None:
        raise ParseError("internal: std::pair without second type", path, p.line, p.col)
    ft = p.cpp_type
    _reject_rect_in_pair_types(ft, st, path, p.line, p.col)
    mem = p.member
    fid = member_to_field_id(p.member)
    label_esc = cpp_escape_string(default_label(p))
    leaf_meta = "Engine::PropertyMeta{}"

    out.append(f'\tb.pushObject("{fid}", "{label_esc}", {meta_arg});')
    for which, wt, nid, label in (
        ("first", ft, "first", "First"),
        ("second", st, "second", "Second"),
    ):
        acc = f"{mem}.{which}"
        g = _scalar_get_lambda(wt, capture="[this]", read_expr=acc, call_syntax=True)
        if readonly:
            s = _scalar_set_lambda(wt, mode="readonly", capture="[this]")
        else:
            s = _scalar_set_lambda(
                wt,
                mode="assign",
                capture="[this]",
                assign_lhs=acc,
                move_policy="move_by_value",
            )
        _emit_scalar_leaf(out, wt, nid, label, g, s, leaf_meta, path, p.line, p.col)
    out.append("\tb.pop();")


def _optional_storage_expr(p: PropSpec) -> str:
    if p.is_getter:
        return f"this->{p.member}()"
    return p.member


def _optional_has_value_get(storage: str) -> str:
    return f"[this]() {{ return {storage}.has_value(); }}"


def _optional_has_value_set(
    p: PropSpec,
    storage: str,
    inner_t: str,
    readonly: bool,
    setter_name: str,
) -> str:
    if readonly:
        return "[this](bool) {}"
    default = _default_cpp_value(inner_t)
    if p.is_getter:
        return (
            f"[this](bool v) {{\n"
            f"\t\tauto _opt = {storage};\n"
            f"\t\tif (v) {{\n"
            f"\t\t\tif (!_opt) _opt = {default};\n"
            f"\t\t}} else {{\n"
            f"\t\t\t_opt = std::nullopt;\n"
            f"\t\t}}\n"
            f"\t\tthis->{setter_name}(std::move(_opt));\n"
            f"\t}}"
        )
    return (
        f"[this](bool v) {{\n"
        f"\t\tif (v) {{\n"
        f"\t\t\tif (!{storage}) {storage} = {default};\n"
        f"\t\t}} else {{\n"
        f"\t\t\t{storage} = std::nullopt;\n"
        f"\t\t}}\n"
        f"\t}}"
    )


def _optional_value_get(storage: str, inner_t: str) -> str:
    default = _default_cpp_value(inner_t)
    if inner_t == "sf::Angle":
        return (
            f"[this]() {{ const auto& _opt = {storage}; "
            f"return _opt ? _opt->asDegrees() : {default}.asDegrees(); }}"
        )
    if inner_t == "int":
        return (
            f"[this]() {{ const auto& _opt = {storage}; "
            f"return _opt ? static_cast<std::int32_t>(*_opt) : static_cast<std::int32_t>({default}); }}"
        )
    return f"[this]() {{ const auto& _opt = {storage}; return _opt ? *_opt : {default}; }}"


def _optional_value_set(
    p: PropSpec,
    storage: str,
    inner_t: str,
    readonly: bool,
    setter_name: str,
) -> str:
    if readonly:
        return _scalar_set_lambda(inner_t, mode="readonly", capture="[this]")
    if p.is_getter:
        return _scalar_set_lambda(
            inner_t,
            mode="call_setter",
            capture="[this]",
            setter_name=setter_name,
            move_policy="move_by_value",
        )
    return _scalar_set_lambda(
        inner_t,
        mode="assign",
        capture="[this]",
        assign_lhs=storage,
        move_policy="move_by_value",
    )


def emit_std_optional_property(
    out: list[str],
    p: PropSpec,
    path: Path,
    meta_arg: str,
    readonly: bool,
) -> None:
    inner_t = p.cpp_type
    setter_raw = p.attrs.get("setter")
    setter_name = setter_raw.strip() if isinstance(setter_raw, str) else ""

    if p.is_getter and not readonly and not setter_name:
        raise ParseError(
            "writable @getter returning std::optional requires a paired @setter (or setter= on @getter)",
            path,
            p.line,
            p.col,
        )

    storage = _optional_storage_expr(p)
    fid = member_to_field_id(p.member)
    label_esc = cpp_escape_string(default_label(p))
    leaf_meta = "Engine::PropertyMeta{}"

    out.append(f'\tb.pushObject("{fid}", "{label_esc}", {meta_arg});')
    g_has = _optional_has_value_get(storage)
    s_has = _optional_has_value_set(p, storage, inner_t, readonly, setter_name)
    out.append(f'\t\tb.addBool("has_value", "Set", {g_has}, {s_has}, {leaf_meta});')
    if _is_rect_type(inner_t):
        vec_t = _rect_vec_type(inner_t)
        if p.is_getter:
            get_pos = _rect_component_get_optional(storage, "position", inner_t)
            get_size = _rect_component_get_optional(storage, "size", inner_t)
            set_pos = _rect_component_set_optional_getter(
                storage, setter_name, "position", inner_t, vec_t, readonly
            )
            set_size = _rect_component_set_optional_getter(
                storage, setter_name, "size", inner_t, vec_t, readonly
            )
        else:
            get_pos = _rect_component_get_optional(storage, "position", inner_t)
            get_size = _rect_component_get_optional(storage, "size", inner_t)
            set_pos = _rect_component_set_optional_field(storage, "position", inner_t, vec_t, readonly)
            set_size = _rect_component_set_optional_field(storage, "size", inner_t, vec_t, readonly)
        _emit_rect_children(
            out,
            inner_t,
            path,
            p.line,
            p.col,
            get_pos=get_pos,
            set_pos=set_pos,
            get_size=get_size,
            set_size=set_size,
        )
    else:
        g_val = _optional_value_get(storage, inner_t)
        s_val = _optional_value_set(p, storage, inner_t, readonly, setter_name)
        _emit_scalar_leaf(out, inner_t, "value", "Value", g_val, s_val, leaf_meta, path, p.line, p.col)
    out.append("\tb.pop();")


def _vector_rect_component_set(
    acc: str,
    idx: str,
    component: str,
    vec_t: str,
    readonly: bool,
    *,
    is_getter: bool,
    vec_access: str,
    setter_name: str,
) -> str:
    if readonly:
        return f"[this, {idx}]({vec_t}) {{}}"
    if is_getter:
        return (
            f"[this, {idx}]({vec_t} v) {{\n"
            f"\t\tauto _pv = {vec_access};\n"
            f"\t\t_pv[{idx}].{component} = v;\n"
            f"\t\tthis->{setter_name}(std::move(_pv));\n"
            f"\t}}"
        )
    return f"[this, {idx}]({vec_t} v) {{ {acc}.{component} = v; }}"


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
    label_esc = cpp_escape_string(default_label(p))
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

    t = p.cpp_type
    if _is_rect_type(t):
        vec_t = _rect_vec_type(t)
        get_pos = f"[this, {idx}]() {{ return {acc}.position; }}"
        get_size = f"[this, {idx}]() {{ return {acc}.size; }}"
        set_pos = _vector_rect_component_set(
            acc,
            idx,
            "position",
            vec_t,
            readonly,
            is_getter=p.is_getter,
            vec_access=vec_access,
            setter_name=setter_name,
        )
        set_size = _vector_rect_component_set(
            acc,
            idx,
            "size",
            vec_t,
            readonly,
            is_getter=p.is_getter,
            vec_access=vec_access,
            setter_name=setter_name,
        )
        _emit_rect_children(
            out,
            t,
            path,
            p.line,
            p.col,
            get_pos=get_pos,
            set_pos=set_pos,
            get_size=get_size,
            set_size=set_size,
        )
    else:
        capture = f"[this, {idx}]"
        g = _scalar_get_lambda(t, capture=capture, read_expr=acc)
        if readonly:
            s = _scalar_set_lambda(t, mode="readonly", capture=capture)
        elif p.is_getter:
            s = _scalar_set_lambda(
                t,
                mode="vector_cow",
                capture=capture,
                vec_access=vec_access,
                vec_index=idx,
                setter_name=setter_name,
            )
        else:
            s = _scalar_set_lambda(
                t,
                mode="assign",
                capture=capture,
                assign_lhs=acc,
                move_policy="string_only",
            )
        _emit_scalar_leaf(out, t, "v", "", g, s, meta_arg, path, p.line, p.col)
    out.append("\t\tb.pop();")
    out.append("\t}")
    out.append("\tb.endSequence();")


def ref_inner_is_scene_node(inner: str) -> bool:
    t = inner.strip()
    return t == "SceneNode" or t.endswith("::SceneNode")


def generate_file_content(
    path: Path, classes: list[ClassSpec], property_base_targets: set[str]
) -> str:
    out: list[str] = [
        "// Generated by tools/property_codegen.py — do not edit.",
        "#pragma once",
        "",
        '#include "Engine/Core/PropertyTree.h"',
        "",
    ]
    if any(
        isinstance(p.attrs.get("valuesProvider"), str) and str(p.attrs.get("valuesProvider", "")).strip()
        for c in classes
        for p in c.props
    ):
        out.append('#include "Engine/Editor/ValuesProviders.h"')
        out.append("")
    out.extend(
        [
            "#include <functional>",
            "",
        ]
    )
    def uses_sfml(p: PropSpec) -> bool:
        if p.is_map:
            mv = p.map_value_type or ""
            return p.cpp_type.startswith("sf::") or mv.startswith("sf::")
        if p.is_pair:
            st = p.map_value_type or ""
            return p.cpp_type.startswith("sf::") or st.startswith("sf::")
        if p.is_set:
            return p.cpp_type.startswith("sf::")
        return p.cpp_type.startswith("sf::")

    needs_sfml = any(uses_sfml(p) for c in classes for p in c.props)
    if needs_sfml:
        out.append('#include <SFML/Graphics/Color.hpp>')
        out.append('#include <SFML/Graphics/Rect.hpp>')
        out.append('#include <SFML/System/Vector2.hpp>')
        out.append('#include <SFML/System/Vector3.hpp>')
        out.append("")
    if any(
        p.cpp_type == "sf::Angle" or p.map_value_type == "sf::Angle"
        for c in classes
        for p in c.props
    ):
        out.append("#include <SFML/System/Angle.hpp>")
        out.append("")
    out.append("#include <cstdint>")
    out.append("#include <string>")
    if any(p.is_vector for c in classes for p in c.props):
        out.append("#include <cstddef>")
    if any(p.is_bitset for c in classes for p in c.props):
        out.append("#include <bitset>")
    if any(p.is_pair for c in classes for p in c.props):
        out.append("#include <utility>")
    if any(p.is_optional for c in classes for p in c.props):
        out.append("#include <optional>")
    if any(p.is_ref_wrapper for c in classes for p in c.props):
        out.append("#include <memory>")
        out.append('#include "Engine/Core/EntityOnNode.h"')
    assoc_headers: set[str] = set()
    reflected_types = {c.qualified() for c in classes}

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
    if any(p.enum_enumerators is not None for c in classes for p in c.props):
        out.append("#include <utility>")
        out.append("#include <vector>")
    out.append("")

    for c in classes:
        q = c.qualified()
        root_id = pascal_to_snake(c.class_name)
        root_label = cpp_escape_string(c.class_name)
        uses_wrapper = class_uses_property_tree_wrapper(c, property_base_targets)
        out.append(f"void {q}::BuildPropertyTree(Engine::PropertyBuilder& b) {{")
        if c.property_base:
            out.append(f"\t{c.property_base}::BuildPropertyTree(b);")
        if uses_wrapper:
            out.append(f'\tb.pushObject("{root_id}", "{root_label}");')
        for p in c.props:
            validate_values_provider_tag(p, path)
            a = p.attrs
            setter_name = a.get("setter")
            has_setter_method = isinstance(setter_name, str) and bool(setter_name)
            readonly = (a.get("readonly") is True) or (p.is_getter and not has_setter_method)

            if p.is_ref_wrapper:
                if p.is_getter:
                    raise ParseError(
                        "RefWrapper fields must be data members (not @getter) in codegen v1",
                        path,
                        p.line,
                        p.col,
                    )
                inner = p.ref_inner_type or ""
                filter_kind = (
                    "Engine::SceneRefFilterKind::SceneNode"
                    if ref_inner_is_scene_node(inner)
                    else "Engine::SceneRefFilterKind::Entity"
                )
                fid = member_to_field_id(p.member)
                label_esc = cpp_escape_string(default_label(p))
                get_lambda = f"[this]() -> std::uint32_t {{ return static_cast<std::uint32_t>(this->{p.member}.GetId()); }}"
                set_lambda = (
                    "[](std::uint32_t) {}"
                    if readonly
                    else f"[this](std::uint32_t v) {{ this->{p.member}.SetId(static_cast<Engine::EntityId>(v)); }}"
                )
                out.append("\t{")
                out.append("\t\tEngine::PropertyMeta _sm{};")
                if readonly:
                    out.append("\t\t_sm.readOnly = true;")
                if isinstance(a.get("tooltip"), str):
                    out.append(f'\t\t_sm.tooltip = "{cpp_escape_string(a["tooltip"])}";')
                out.append(f"\t\t_sm.sceneRefFilterKind = {filter_kind};")
                if not ref_inner_is_scene_node(inner):
                    out.append(
                        f"\t\t_sm.sceneRefEntityIsAllowed = [](const std::shared_ptr<EntityOnNode>& e) -> bool "
                        f"{{ return static_cast<bool>(std::dynamic_pointer_cast<{inner}>(e)); }};"
                    )
                out.append(f'\t\tb.addSceneRef("{fid}", "{label_esc}", {get_lambda}, {set_lambda}, _sm);')
                out.append("\t}")
                continue

            if p.is_bitset:
                if p.is_getter:
                    raise ParseError(
                        "std::bitset properties must be data members (not @getter) in codegen v1",
                        path,
                        p.line,
                        p.col,
                    )
                has_meta = readonly or any(
                    k in a for k in ("tooltip", "minValue", "maxValue", "dragSpeed", "valuesProvider")
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
                    k in a for k in ("tooltip", "minValue", "maxValue", "dragSpeed", "valuesProvider")
                )
                meta_arg = "Engine::PropertyMeta{}" if not has_meta else format_meta_inline(p)
                if p.is_map:
                    emit_assoc_map_property(out, p, path, meta_arg, readonly)
                else:
                    emit_assoc_set_property(out, p, path, meta_arg, readonly)
                continue

            if p.is_pair:
                if p.is_getter:
                    raise ParseError(
                        "std::pair fields must be data members (not @getter) in codegen v1",
                        path,
                        p.line,
                        p.col,
                    )
                has_meta = readonly or any(
                    k in a for k in ("tooltip", "minValue", "maxValue", "dragSpeed", "valuesProvider")
                )
                meta_arg = "Engine::PropertyMeta{}" if not has_meta else format_meta_inline(p)
                emit_std_pair_property(out, p, path, meta_arg, readonly)
                continue

            if p.is_optional:
                has_meta = readonly or any(
                    k in a for k in ("tooltip", "minValue", "maxValue", "dragSpeed", "valuesProvider")
                )
                meta_arg = "Engine::PropertyMeta{}" if not has_meta else format_meta_inline(p)
                emit_std_optional_property(out, p, path, meta_arg, readonly)
                continue

            if p.enum_enumerators is not None:
                has_meta = readonly or any(
                    k in a for k in ("tooltip", "minValue", "maxValue", "dragSpeed", "valuesProvider")
                )
                meta_arg = "Engine::PropertyMeta{}" if not has_meta else format_meta_inline(p)
                canon = p.cpp_type
                if p.is_getter:
                    get_lambda = f"[this]() {{ return static_cast<int>(this->{p.member}()); }}"
                else:
                    get_lambda = f"[this]() {{ return static_cast<int>({p.member}); }}"
                sn = setter_name if isinstance(setter_name, str) else ""
                if p.is_getter and has_setter_method:
                    set_lambda = f"[this](int v) {{ this->{sn}(static_cast<{canon}>(v)); }}"
                elif readonly:
                    set_lambda = "[this](int) {}"
                else:
                    set_lambda = f"[this](int v) {{ {p.member} = static_cast<{canon}>(v); }}"
                fid = member_to_field_id(p.member)
                label_esc = cpp_escape_string(default_label(p))
                opts = ", ".join(
                    f'{{ static_cast<int>({canon}::{ev}), "{cpp_escape_string(ev)}" }}'
                    for ev in p.enum_enumerators
                )
                out.append(
                    f'\tb.addEnum("{fid}", "{label_esc}", {get_lambda}, {set_lambda}, {{ {opts} }}, {meta_arg});'
                )
                continue

            if p.is_object:
                if p.is_getter:
                    raise ParseError(
                        "nested object properties must be data members (not @getter) in codegen v1",
                        path,
                        p.line,
                        p.col,
                    )
                resolved_object_type = resolve_reflected_object_type(
                    p.cpp_type, c.namespaces, reflected_types
                )
                if resolved_object_type is None:
                    raise ParseError(
                        f"unsupported nested object type `{p.cpp_type}`; add META_CLASS() to the type in the same header or use supported scalar/container field type",
                        path,
                        p.line,
                        p.col,
                    )
                has_meta = readonly or any(
                    k in a for k in ("tooltip", "minValue", "maxValue", "dragSpeed", "valuesProvider")
                )
                meta_arg = "Engine::PropertyMeta{}" if not has_meta else format_meta_inline(p)
                fid = member_to_field_id(p.member)
                label_esc = cpp_escape_string(default_label(p))
                out.append(f'\tb.pushObject("{fid}", "{label_esc}", {meta_arg});')
                out.append(f"\tthis->{p.member}.BuildPropertyTree(b);")
                out.append("\tb.pop();")
                continue

            if p.cpp_type not in KNOWN_TYPES:
                raise ParseError(f"unsupported type `{p.cpp_type}`", path, p.line, p.col)

            if _is_rect_type(p.cpp_type):
                has_meta = readonly or any(
                    k in a for k in ("tooltip", "minValue", "maxValue", "dragSpeed", "valuesProvider")
                )
                meta_arg = "Engine::PropertyMeta{}" if not has_meta else format_meta_inline(p)
                sn = setter_name if isinstance(setter_name, str) else ""
                emit_rect_property(out, p, path, meta_arg, readonly, sn)
                continue

            if p.is_vector:
                has_meta = readonly or any(
                    k in a
                    for k in (
                        "tooltip",
                        "minValue",
                        "maxValue",
                        "dragSpeed",
                        "minCount",
                        "maxCount",
                        "valuesProvider",
                    )
                )
                meta_arg = "Engine::PropertyMeta{}" if not has_meta else format_meta_inline(p)
                emit_std_vector_property(out, p, path, meta_arg, readonly)
                continue

            has_meta = readonly or any(
                k in a for k in ("tooltip", "minValue", "maxValue", "dragSpeed", "valuesProvider")
            )
            meta_arg = "Engine::PropertyMeta{}" if not has_meta else format_meta_inline(p)
            t = p.cpp_type
            if p.is_getter:
                read_expr = f"this->{p.member}()"
                call_syntax = True
            else:
                read_expr = p.member
                call_syntax = t in ("int", "sf::Angle")
            get_lambda = _scalar_get_lambda(
                t, capture="[this]", read_expr=read_expr, call_syntax=call_syntax
            )
            setter_method = a.get("setter")
            if p.is_getter and has_setter_method:
                sn = setter_name if isinstance(setter_name, str) else ""
                set_lambda = _scalar_set_lambda(
                    t,
                    mode="call_setter",
                    capture="[this]",
                    setter_name=sn,
                    move_policy="none",
                )
            elif (
                isinstance(setter_method, str)
                and setter_method
                and not readonly
                and not p.is_getter
                and t == "bool"
            ):
                set_lambda = f"[this](bool v) {{ this->{setter_method}(v); }}"
            elif readonly:
                set_lambda = _scalar_set_lambda(t, mode="readonly", capture="[this]")
            else:
                set_lambda = _scalar_set_lambda(
                    t,
                    mode="assign",
                    capture="[this]",
                    assign_lhs=p.member,
                    move_policy="string_only",
                )
            fid = member_to_field_id(p.member)
            label_esc = cpp_escape_string(default_label(p))
            _emit_scalar_leaf(
                out, t, fid, label_esc, get_lambda, set_lambda, meta_arg, path, p.line, p.col, indent="\t"
            )
        for m in c.methods:
            label_esc = cpp_escape_string(method_menu_label(m))
            out.append(f'\tb.registerInspectorMethod("{label_esc}", [this]() {{ this->{m.method}(); }});')
        if uses_wrapper:
            out.append("\tb.pop();")
        out.append("}")
        out.append("")

    return "\n".join(out).rstrip() + "\n"


def should_register_scene_entity(rel_src: str, c: ClassSpec) -> bool:
    """Whether this META_CLASS type participates in scene XML entity registry."""
    n = c.class_name
    if rel_src == "Engine/Core/SceneNode.h":
        return False
    if rel_src.startswith("Engine/Background/"):
        return False
    if rel_src == "Engine/Visual/Visual.h" and n == "Visual":
        return False
    if n == "ShapeVisualBase":
        return False
    if rel_src == "Engine/Core/Transform.h" and n == "Transform":
        return False
    if rel_src.startswith("Engine/Sorting/"):
        if n == "SortingStrategy":
            return False
        return True
    if rel_src.startswith("Engine/Visual/"):
        return True
    if rel_src.startswith("Engine/Behaviour/"):
        return True
    if rel_src.startswith("LaunchProfiles/"):
        return True
    return False


def scene_entity_type_id(rel_src: str, class_name: str) -> str:
    """Stable string id stored in XML (matches prior hand-picked ids)."""
    parts = rel_src.split("/")
    if parts and parts[0] == "Engine":
        return f"Engine.{class_name}"
    if len(parts) >= 2 and parts[0] == "LaunchProfiles":
        return f"{parts[1]}.{class_name}"
    return class_name


def scene_entity_kind_cpp(rel_src: str, class_name: str) -> str:
    """C++ enumerator name for Engine::Serialization::SceneEntityKind."""
    if class_name == "Transform" and rel_src == "Engine/Core/Transform.h":
        return "Transform"
    if rel_src.startswith("Engine/Sorting/"):
        return "SortingStrategy"
    if rel_src.startswith("Engine/Visual/"):
        return "Visual"
    return "Behaviour"


def collect_scene_entity_registry_entries(root: Path) -> list[tuple[ClassSpec, str]]:
    """(ClassSpec, rel_header_posix) sorted for stable output."""
    src = root / "src"
    by_qualified: dict[str, tuple[ClassSpec, str]] = {}
    for h in sorted(src.rglob("*.h"), key=lambda p: p.as_posix()):
        if "Codegen" in h.parts:
            continue
        rel_header = h.relative_to(src).as_posix()
        classes, _ = parse_header(h)
        for c in classes:
            if should_register_scene_entity(rel_header, c):
                by_qualified[c.qualified()] = (c, rel_header)
    out = list(by_qualified.values())
    out.sort(
        key=lambda t: (
            scene_entity_type_id(t[1], t[0].class_name),
            t[0].qualified(),
        )
    )
    return out


def generate_scene_entity_registry_cpp(root: Path) -> str:
    entries = collect_scene_entity_registry_entries(root)
    lines: list[str] = [
        "// Auto-generated by tools/property_codegen.py — do not edit.",
        '#include "Engine/Serialization/SceneEntityRegistrar.h"',
        "",
    ]
    includes = sorted({rel_h for _, rel_h in entries})
    for inc in includes:
        lines.append(f'#include "{inc}"')
    lines.append("")
    lines.append("namespace {")
    lines.append("")
    lines.append("struct TeCodegenSceneEntityRegistration {")
    lines.append("\tTeCodegenSceneEntityRegistration() {")
    lines.append("\t\tusing ::Engine::Serialization::RegisterSceneEntity;")
    lines.append("\t\tusing ::Engine::Serialization::SceneEntityKind;")
    for c, rel_h in entries:
        q = c.qualified()
        tid = scene_entity_type_id(rel_h, c.class_name)
        tid_esc = cpp_escape_string(tid)
        kind = scene_entity_kind_cpp(rel_h, c.class_name)
        lines.append(f'\t\tRegisterSceneEntity<{q}>("{tid_esc}", SceneEntityKind::{kind});')
    lines.append("\t}")
    lines.append("};")
    lines.append("")
    lines.append("TeCodegenSceneEntityRegistration teCodegenSceneEntityRegistration{};")
    lines.append("")
    lines.append("} // namespace")
    lines.append("")
    return "\n".join(lines)


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


def run_scalar_lambda_self_tests() -> int:
    def check(label: str, got: str, want: str) -> bool:
        if got != want:
            print(
                f"[codegen] scalar lambda test FAIL {label}: got {got!r} expected {want!r}",
                file=sys.stderr,
            )
            return False
        return True

    ok = True
    want = "[this]() { return static_cast<std::int32_t>(_priority); }"
    got = _scalar_get_lambda("int", capture="[this]", read_expr="_priority", call_syntax=True)
    ok = check("top-level int field get", got, want) and ok

    want = "[this]() { return this->GetPointCount(); }"
    got = _scalar_get_lambda(
        "int", capture="[this]", read_expr="this->GetPointCount()", call_syntax=True
    )
    # int getter also gets cast when using unified helper
    want_int_getter = (
        "[this]() { return static_cast<std::int32_t>(this->GetPointCount()); }"
    )
    ok = check("top-level int getter", got, want_int_getter) and ok

    want = "[this](float v) { _mass = v; }"
    got = _scalar_set_lambda(
        "float",
        mode="assign",
        capture="[this]",
        assign_lhs="_mass",
        move_policy="string_only",
    )
    ok = check("top-level float field set", got, want) and ok

    want = "[this, _i](float) {}"
    got = _scalar_set_lambda("float", mode="readonly", capture="[this, _i]")
    ok = check("vector readonly float", got, want) and ok

    want = (
        "[this, _i](float v) { auto _pv = _items; _pv[_i] = v; "
        "this->SetItems(std::move(_pv)); }"
    )
    got = _scalar_set_lambda(
        "float",
        mode="vector_cow",
        capture="[this, _i]",
        vec_access="_items",
        vec_index="_i",
        setter_name="SetItems",
    )
    ok = check("vector cow float", got, want) and ok

    want = "[this](sf::Vector2f v) { _pair.first = std::move(v); }"
    got = _scalar_set_lambda(
        "sf::Vector2f",
        mode="assign",
        capture="[this]",
        assign_lhs="_pair.first",
        move_policy="move_by_value",
    )
    ok = check("pair Vector2f assign", got, want) and ok

    want = "[this](std::int32_t) {}"
    got = _scalar_set_lambda("int", mode="readonly", capture="[this]")
    ok = check("readonly int", got, want) and ok

    want = "[this](float) {}"
    got = _scalar_set_lambda("sf::Angle", mode="readonly", capture="[this]")
    ok = check("readonly Angle", got, want) and ok

    if ok:
        print("[codegen] scalar lambda self-tests OK")
        return 0
    return 1


def run_rect_codegen_self_tests() -> int:
    path = Path("self_test.h")
    try:
        _reject_rect_in_pair_types("sf::IntRect", "int", path, 1, 1)
        print("[codegen] rect self-test: expected ParseError for pair+IntRect", file=sys.stderr)
        return 1
    except ParseError as e:
        if "std::pair" not in str(e):
            print(f"[codegen] rect self-test: unexpected error: {e}", file=sys.stderr)
            return 1
    try:
        _reject_rect_in_pair_types("float", "int", path, 1, 1)
    except ParseError as e:
        print(f"[codegen] rect self-test: unexpected error for valid pair: {e}", file=sys.stderr)
        return 1
    if _rect_vec_type("sf::IntRect") != "sf::Vector2i":
        print("[codegen] rect self-test: IntRect component type mismatch", file=sys.stderr)
        return 1
    if _rect_vec_type("sf::FloatRect") != "sf::Vector2f":
        print("[codegen] rect self-test: FloatRect component type mismatch", file=sys.stderr)
        return 1
    body = _rect_grid_add_body("mem", "sf::IntRect", "_nk", "mem.emplace(_nk, dv);", "\t\t\t")
    if "sf::IntRect _nk" not in body or "_placed" not in body:
        print("[codegen] rect self-test: grid add body missing expected fragments", file=sys.stderr)
        return 1
    print("[codegen] rect type self-tests OK")
    return 0


def run_label_inference_self_tests() -> int:
    cases: list[tuple[tuple[str, bool, dict[str, Any]], str]] = [
        (("_thisIsAProperty", False, {}), "This is a property"),
        (("_angularSpeed", False, {}), "Angular speed"),
        (("_gravityScale", False, {}), "Gravity scale"),
        (("_interactionGroups", False, {}), "Interaction groups"),
        (("_mass", False, {}), "Mass"),
        (("GetSomeProp", True, {}), "Some prop"),
        (("GetPointCount", True, {}), "Point count"),
        (("GetMass", True, {}), "Mass"),
        (("GetHTTPPort", True, {}), "Http port"),
        (("_angle", False, {"name": "Angle (rad)"}), "Angle (rad)"),
        (("GetPoints", True, {"name": "Points"}), "Points"),
        (("getLocalPosition", True, {}), "Local position"),
    ]
    for (member, is_getter, attrs), want in cases:
        got = inferred_display_label(member, attrs, is_getter=is_getter)
        if got != want:
            print(
                f"[codegen] label test FAIL member={member!r} is_getter={is_getter} attrs={attrs!r}: "
                f"got {got!r} expected {want!r}",
                file=sys.stderr,
            )
            return 1
    print("[codegen] label inference self-tests OK")
    return 0


def main() -> int:
    ap = argparse.ArgumentParser(description="Property tree codegen for TestEngine.")
    ap.add_argument("--root", type=Path, default=Path.cwd(), help="Repository root (default: cwd)")
    ap.add_argument("--force", action="store_true", help="Ignore cache and regenerate all headers")
    ap.add_argument("--verbose", action="store_true", help="Log every scanned header (default: summary only)")
    ap.add_argument(
        "--test-labels",
        action="store_true",
        help="Run display-label heuristics self-tests and exit (no scan)",
    )
    args = ap.parse_args()
    if args.test_labels:
        rc = run_rect_codegen_self_tests()
        if rc != 0:
            return rc
        rc = run_scalar_lambda_self_tests()
        if rc != 0:
            return rc
        return run_label_inference_self_tests()
    root: Path = args.root.resolve()
    src = root / "src"
    codegen_dir = src / "Codegen"
    cache_path = codegen_dir / CACHE_NAME

    if not src.is_dir():
        print(f"[codegen] ERROR: missing src directory: {src}", file=sys.stderr)
        return 1

    codegen_dir.mkdir(parents=True, exist_ok=True)

    try:
        property_base_targets = collect_property_base_targets(root)
    except ParseError as e:
        print(f"[codegen] ERROR: {e}", file=sys.stderr)
        return 1

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
            content = generate_file_content(h, meta_only, property_base_targets)
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

    registry_path = codegen_dir / "SceneEntityRegistry.generated.cpp"
    try:
        reg_content = generate_scene_entity_registry_cpp(root)
    except ParseError as e:
        print(f"[codegen] ERROR: {e}", file=sys.stderr)
        return 1
    old_reg = registry_path.read_text(encoding="utf-8") if registry_path.is_file() else None
    if old_reg != reg_content:
        registry_path.write_text(reg_content, encoding="utf-8")
        print(f"[codegen] wrote {registry_path.relative_to(root)}")
    else:
        print(f"[codegen] unchanged {registry_path.relative_to(root)}")

    cache_files = {k: cache_files[k] for k in sorted(cache_files)}
    save_cache(cache_path, {"version": CACHE_VERSION, "files": cache_files})

    wrapper_ids = collect_inheritance_property_wrapper_ids(root)
    scene_path = root / "assets" / "sceneObjects" / "CurrentScene.xml"
    migrate_passes = migrate_scene_property_wrappers(scene_path, wrapper_ids)
    if migrate_passes:
        print(
            f"[codegen] flattened inheritance property wrappers in "
            f"{scene_path.relative_to(root)} ({migrate_passes} pass(es))"
        )

    print(
        f"[codegen] done. headers_with_meta={touched}, skipped_cached={skipped_cache}, "
        f"cache={cache_path.relative_to(root)}"
    )
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
