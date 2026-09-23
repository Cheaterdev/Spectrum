"""
Generates Visual Studio editor support for .sig files from SIG.g4.

SIG.g4 stays the only grammar anyone edits. Everything lexical comes from it:
keyword sets, declaration keywords, token regexes (comments, numbers, strings,
backtick raw spans, %{ }% blocks), operators and bracket pairs. What the .g4
cannot say -- which TextMate scope a token gets, and which spans embed another
language -- is the small table in SCOPES below.

Outputs (bin/editor/):
  sig.vsix                         install by double-clicking
  folder/SIG/...                   same content as a no-VSIX install; see --install-user

Usage:
  python gen_vs_extension.py                 # regenerate bin/editor/
  python gen_vs_extension.py --install-user  # also copy into %USERPROFILE%/.vs/Extensions/SIG
"""

import argparse
import json
import os
import re
import shutil
import sys
import time
import zipfile

HERE = os.path.dirname(os.path.abspath(__file__))
G4_PATH = os.path.normpath(os.path.join(HERE, "..", "SIG.g4"))
OUT_DIR = os.path.normpath(os.path.join(HERE, "..", "..", "..", "bin", "editor"))

EXT_ID = "Spectrum.SIG.Language"
EXT_NAME = "SIG language (Spectrum)"
SCOPE = "source.sig"

# --- the only hand-maintained part --------------------------------------------

# Lexer token -> scope, plus an optional embedded grammar for begin/end spans.
# VS ships no HLSL TextMate grammar; source.cpp is the closest one it does ship.
SCOPES = {
    "COMMENT":      {"name": "comment.line.number-sign.sig"},
    "STRING":       {"name": "string.quoted.double.sig"},
    "RAWEXPR":      {"name": "string.interpolated.raw.sig", "embed": "source.cpp"},
    "INSERT_BLOCK": {"name": "meta.embedded.block.hlsl.sig", "embed": "source.cpp"},
    "FLOAT_SCALAR": {"name": "constant.numeric.float.sig"},
    "INT_SCALAR":   {"name": "constant.numeric.integer.sig"},
}

# Parser rules that are pure literal alternatives -> scope for those words.
KEYWORD_GROUP_SCOPES = {
    "shader_type":   "support.type.shader-stage.sig",
    "pso_param_id":  "variable.parameter.sig",
    "node_param_id": "variable.parameter.sig",
    "bool_type":     "constant.language.sig",
}

# Built-in type names. The .g4 cannot supply these: every type is a plain ID
# there, and what makes one "built-in" is what Parsed.cpp's detect_type and the
# FrameGraph accept.
#
# Scalars are HLSL base names; the pattern adds the vector/matrix suffixes, so
# `float` also covers float3 / float4x4 and float16_t covers float16_t2.
BUILTIN_SCALARS = [
    "bool", "int", "uint", "dword", "half", "float", "double",
    "min16float", "min10float", "min16int", "min12int", "min16uint",
    "int16_t", "uint16_t", "int32_t", "uint32_t", "int64_t", "uint64_t",
    "float16_t", "float32_t", "float64_t",
]
BUILTIN_KEYWORD_TYPES = ["matrix", "vector", "unorm", "snorm", "mat4x4"]  # mat4x4: detect_type
BUILTIN_RESOURCES = [
    # HLSL
    "Texture1D", "Texture1DArray", "Texture2D", "Texture2DArray", "Texture2DMS", "Texture2DMSArray",
    "Texture3D", "TextureCube", "TextureCubeArray",
    "RWTexture1D", "RWTexture1DArray", "RWTexture2D", "RWTexture2DArray", "RWTexture3D",
    "Buffer", "RWBuffer", "ByteAddressBuffer", "RWByteAddressBuffer",
    "StructuredBuffer", "RWStructuredBuffer", "AppendStructuredBuffer", "ConsumeStructuredBuffer",
    "ConstantBuffer", "SamplerState", "SamplerComparisonState", "RaytracingAccelerationStructure",
    "FeedbackTexture2D", "FeedbackTexture2DArray", "FeedbackTexture2DMip",
    # SIG resource kinds (detect_type) and FrameGraph handler types (FrameGraph.Base.ixx;
    # ByteAdressBuffer is spelled that way there).
    "Texture", "DepthStencil", "RenderTarget", "FormattedBuffer", "ByteAdressBuffer",
]

# TextMate scope selector -> VS classification name. VS colours nothing it has
# no mapping for: without this file only keyword/comment/number showed up.
# Classification names are VS's own, restricted to ones the starter-kit themes
# already use, so they exist even without C#/Roslyn installed. The .sig-suffixed
# selectors are more specific than the generic ones, which exist for the C++
# grammar embedded in %{ }% and backtick spans.
THEME = [
    ("comment, punctuation.definition.comment", "comment"),
    ("string, punctuation.definition.string, constant.character.escape", "string"),
    ("constant.numeric", "number"),
    ("keyword, storage, constant.language, variable.language", "keyword"),
    ("keyword.operator, punctuation.separator", "operator"),
    ("meta.preprocessor keyword", "preprocessor keyword"),
    ("entity.name.type, support.type, entity.name.class", "class name"),
    ("entity.name.function, support.function", "method name"),
    ("variable.parameter", "local name"),
    ("variable.other.member", "enum name"),
    # SIG-specific
    ("storage.type.sig, keyword.other.sig, constant.language.sig", "keyword"),
    ("entity.name.type.sig", "class name"),
    ("variable.other.member.sig", "enum name"),
    ("entity.other.attribute-name.sig", "method name"),
    ("support.type.shader-stage.sig", "preprocessor keyword"),
    ("variable.parameter.sig", "local name"),
    ("punctuation.definition.attribute, punctuation.section.embedded", "operator"),
]

DECL_KEYWORD_SCOPE = "storage.type.sig"
OTHER_KEYWORD_SCOPE = "keyword.other.sig"
OPERATOR_SCOPE = "keyword.operator.sig"

# ------------------------------------------------------------------------------


def strip_g4_comments(text):
    out, i, n = [], 0, len(text)
    while i < n:
        c = text[i]
        if c == "'":
            j = i + 1
            while j < n and text[j] != "'":
                j += 2 if text[j] == "\\" else 1
            out.append(text[i:j + 1])
            i = j + 1
        elif text.startswith("//", i):
            while i < n and text[i] != "\n":
                i += 1
        elif text.startswith("/*", i):
            end = text.find("*/", i + 2)
            i = n if end < 0 else end + 2
        else:
            out.append(c)
            i += 1
    return "".join(out)


def split_rules(text):
    """Returns {name: body} for every `name : body ;` rule."""
    text = strip_g4_comments(text)
    text = re.sub(r"^\s*grammar\s+\w+\s*;", "", text, flags=re.M)
    text = re.sub(r"options\s*\{[^}]*\}", "", text)

    rules, i, n = {}, 0, len(text)
    while i < n:
        m = re.compile(r"\s*([A-Za-z_]\w*)\s*:").match(text, i)
        if not m:
            i += 1
            continue
        name, j = m.group(1), m.end()
        k = j
        while k < n and text[k] != ";":
            if text[k] == "'":
                k += 1
                while k < n and text[k] != "'":
                    k += 2 if text[k] == "\\" else 1
            elif text[k] == "[":
                while k < n and text[k] != "]":
                    k += 2 if text[k] == "\\" else 1
            k += 1
        rules[name] = text[j:k].strip()
        i = k + 1
    return rules


# --- ANTLR lexer rule -> Oniguruma regex ---------------------------------------

TOKEN_RE = re.compile(r"""
    (?P<lit>'(?:\\.|[^'\\])*')
  | (?P<set>~?\[(?:\\.|[^\]\\])*\])
  | (?P<cmd>->.*$)
  | (?P<ref>[A-Z_][A-Za-z_0-9]*)
  | (?P<op>[()|*+?.~])
  | (?P<ws>\s+)
""", re.X | re.S)


def unescape_literal(lit):
    body = lit[1:-1]
    return re.sub(r"\\(u[0-9a-fA-F]{4}|.)",
                  lambda m: chr(int(m.group(1)[1:], 16)) if m.group(1)[0] == "u"
                  else {"n": "\n", "r": "\r", "t": "\t"}.get(m.group(1), m.group(1)), body)


def re_escape(s):
    return "".join(c if c.isalnum() or c == "_" else "\\" + c for c in s)


def set_to_regex(s):
    neg = s.startswith("~")
    inner = s[2:-1] if neg else s[1:-1]
    # ANTLR set escapes are regex-class escapes already, except a bare `[`/`^`.
    inner = inner.replace("[", "\\[")
    if inner.startswith("^"):
        inner = "\\" + inner
    return ("[^" if neg else "[") + inner + "]"


def tokenize(body):
    toks = []
    for m in TOKEN_RE.finditer(body):
        kind = m.lastgroup
        if kind in ("ws", "cmd"):
            continue
        toks.append((kind, m.group(kind)))
    return toks


def lexer_ast(name, rules, depth=0):
    """Parses a lexer rule into a tiny AST: ('alt', [seq...]) of ('seq', [atoms])."""
    if depth > 20:
        raise ValueError("lexer rule recursion: " + name)
    toks = tokenize(rules[name])
    pos = 0

    def parse_alt():
        nonlocal pos
        seqs = [parse_seq()]
        while pos < len(toks) and toks[pos] == ("op", "|"):
            pos += 1
            seqs.append(parse_seq())
        return ("alt", seqs)

    def parse_seq():
        nonlocal pos
        atoms = []
        while pos < len(toks) and toks[pos] not in (("op", "|"), ("op", ")")):
            atom = parse_atom()
            while pos < len(toks) and toks[pos][0] == "op" and toks[pos][1] in "*+?":
                q = toks[pos][1]
                pos += 1
                lazy = False
                if q in "*+" and pos < len(toks) and toks[pos] == ("op", "?"):
                    lazy = True
                    pos += 1
                atom = ("rep", atom, q, lazy)
            atoms.append(atom)
        return ("seq", atoms)

    def parse_atom():
        nonlocal pos
        kind, val = toks[pos]
        pos += 1
        if kind == "lit":
            return ("lit", unescape_literal(val))
        if kind == "set":
            return ("set", val)
        if kind == "ref":
            return lexer_ast(val, rules, depth + 1)
        if val == "(":
            inner = parse_alt()
            pos += 1  # ')'
            return inner
        if val == ".":
            return ("any",)
        raise ValueError(f"unsupported lexer syntax in {name}: {val}")

    return parse_alt()


def to_regex(node):
    kind = node[0]
    if kind == "lit":
        return re_escape(node[1])
    if kind == "set":
        return set_to_regex(node[1])
    if kind == "any":
        return "[\\s\\S]"
    if kind == "rep":
        inner = to_regex(node[1])
        if len(inner) > 1 and not (inner.startswith("[") and inner.endswith("]") and inner.count("[") == 1) \
                and not (inner.startswith("(?:") and inner.endswith(")")):
            inner = "(?:" + inner + ")"
        return inner + node[2] + ("?" if node[3] else "")
    if kind == "seq":
        return "".join(to_regex(a) for a in node[1])
    if kind == "alt":
        parts = [to_regex(s) for s in node[1]]
        return parts[0] if len(parts) == 1 else "(?:" + "|".join(parts) + ")"
    raise ValueError(kind)


def flatten(node):
    """Collapses single-branch alt/seq wrappers."""
    while node[0] in ("alt", "seq") and len(node[1]) == 1:
        node = node[1][0]
    return node


def literal_of(node):
    node = flatten(node)
    return node[1] if node[0] == "lit" else None


def begin_end(node):
    """For `START body END` shaped rules, returns (start, end) literals."""
    node = flatten(node)
    if node[0] != "seq" or len(node[1]) < 3:
        return None
    first, last = literal_of(node[1][0]), literal_of(node[1][-1])
    return (first, last) if first and last else None


# --- grammar analysis ----------------------------------------------------------

def is_lexer(name):
    return name[0].isupper()


def literal_rules(rules):
    """Lexer rules whose whole body is one literal: {NAME: text}."""
    out = {}
    for name, body in rules.items():
        if is_lexer(name):
            m = re.fullmatch(r"'((?:\\.|[^'\\])*)'", body.strip())
            if m:
                out[name] = unescape_literal("'" + m.group(1) + "'")
    return out


def words_in(body, lit_rules):
    """Keyword words referenced by a parser rule body: literals and literal-token refs."""
    words = []
    for kind, val in tokenize(body):
        if kind == "lit":
            w = unescape_literal(val)
        elif kind == "ref" and val in lit_rules:
            w = lit_rules[val]
        else:
            continue
        if re.fullmatch(r"[A-Za-z_]\w*", w):
            words.append(w)
    return words


def analyse(rules):
    lit_rules = literal_rules(rules)

    groups = {}
    for rule, scope in KEYWORD_GROUP_SCOPES.items():
        if rule in rules:
            groups.setdefault(scope, set()).update(words_in(rules[rule], lit_rules))

    # A declaration keyword is the word that opens a *_definition rule.
    decl = set()
    for name, body in rules.items():
        if not is_lexer(name) and name.endswith("_definition"):
            for alt in body.split("|"):
                ws = [w for w in words_in(alt, lit_rules)]
                if ws:
                    decl.add(ws[0])

    grouped = set().union(*groups.values()) if groups else set()
    all_words = set()
    for name, body in rules.items():
        if not is_lexer(name):
            all_words.update(words_in(body, lit_rules))
    other = all_words - decl - grouped

    symbolic = {n: t for n, t in lit_rules.items() if not re.fullmatch(r"[A-Za-z_]\w*", t)}

    pairs = []
    for n, t in symbolic.items():
        closer = None
        if n.startswith("O") and ("C" + n[1:]) in symbolic:
            closer = symbolic["C" + n[1:]]
        elif n.endswith("_START") and (n[:-6] + "_END") in symbolic:
            closer = symbolic[n[:-6] + "_END"]
        if closer:
            pairs.append([t, closer])
    bracket_chars = {c for p in pairs for c in p}
    operators = sorted({t for t in symbolic.values() if t not in bracket_chars}, key=len, reverse=True)

    # Also symbolic literals written inline in parser rules (e.g. '::', ',').
    for name, body in rules.items():
        if not is_lexer(name):
            for kind, val in tokenize(body):
                if kind == "lit":
                    t = unescape_literal(val)
                    if not re.fullmatch(r"[A-Za-z_]\w*", t) and t not in bracket_chars and t not in operators:
                        operators.append(t)
    operators.sort(key=len, reverse=True)

    return {
        "lit_rules": lit_rules,
        "groups": groups,
        "decl": decl,
        "other": other,
        "pairs": pairs,
        "operators": operators,
    }


def words_regex(words):
    return r"\b(?:" + "|".join(sorted((re_escape(w) for w in words), key=len, reverse=True)) + r")\b"


def build_grammar(rules, a):
    ident = to_regex(lexer_ast("ID", rules))
    repo = {}
    token_patterns = []

    for tok, cfg in SCOPES.items():
        if tok not in rules:
            continue
        ast = lexer_ast(tok, rules)
        be = begin_end(ast)
        if be and ("embed" in cfg or "[\\s\\S]" in to_regex(ast)):
            # Leading \s* on the end: the embedded C++ grammar has rules that
            # start matching at the indentation before `}%`, and the earliest
            # match wins -- without it the block never closes.
            p = {"name": cfg["name"], "begin": re_escape(be[0]), "end": r"\s*" + re_escape(be[1]),
                 "beginCaptures": {"0": {"name": "punctuation.section.embedded.begin.sig"}},
                 "endCaptures": {"0": {"name": "punctuation.section.embedded.end.sig"}}}
            if "embed" in cfg:
                p["contentName"] = cfg["embed"].replace("source.", "meta.embedded.") + ".sig"
                p["patterns"] = [{"include": cfg["embed"]}]
        else:
            rx = to_regex(ast)
            if tok in ("INT_SCALAR", "FLOAT_SCALAR"):
                rx = r"(?<![\w.])" + rx + r"(?![\w])"
            p = {"name": cfg["name"], "match": rx}
        repo[tok.lower()] = p
        token_patterns.append(tok.lower())

    # Float before int, so `1.5` is one token.
    order = ["comment", "insert_block", "rawexpr", "string", "float_scalar", "int_scalar"]
    token_patterns.sort(key=lambda t: order.index(t) if t in order else len(order))

    repo["declaration"] = {
        "match": "(" + words_regex(a["decl"]) + r")\s+(" + ident + ")",
        "captures": {"1": {"name": DECL_KEYWORD_SCOPE}, "2": {"name": "entity.name.type.sig"}},
    }
    repo["qualified"] = {
        "match": "(" + ident + r")\s*(::)\s*(" + ident + ")",
        "captures": {"1": {"name": "entity.name.type.sig"},
                     "2": {"name": "punctuation.separator.scope.sig"},
                     "3": {"name": "variable.other.member.sig"}},
    }
    repo["function"] = {"match": "(" + ident + r")(?=\s*\()",
                        "captures": {"1": {"name": "entity.name.function.sig"}}}

    kw = []
    for scope, words in sorted(a["groups"].items()):
        key = "kw_" + scope.split(".")[0] + "_" + scope.split(".")[1]
        repo[key] = {"name": scope, "match": words_regex(words)}
        kw.append(key)
    scalars = "|".join(sorted(BUILTIN_SCALARS, key=len, reverse=True))
    repo["builtin_scalar"] = {"name": "storage.type.primitive.sig",
                              "match": r"\b(?:" + scalars + r")(?:[1-4](?:x[1-4])?)?\b"}
    repo["builtin_keyword_type"] = {"name": "storage.type.primitive.sig", "match": words_regex(BUILTIN_KEYWORD_TYPES)}
    repo["builtin_resource"] = {"name": "support.type.resource.sig", "match": words_regex(BUILTIN_RESOURCES)}
    kw = ["builtin_scalar", "builtin_keyword_type", "builtin_resource"] + kw

    repo["kw_decl"] = {"name": DECL_KEYWORD_SCOPE, "match": words_regex(a["decl"])}
    repo["kw_other"] = {"name": OTHER_KEYWORD_SCOPE, "match": words_regex(a["other"])}
    kw += ["kw_decl", "kw_other"]

    # Consumes plain identifiers so digits inside names (R8G8B8A8_UNORM) are
    # not coloured as numbers.
    repo["identifier"] = {"match": ident}
    repo["operator"] = {"name": OPERATOR_SCOPE, "match": "|".join(re_escape(o) for o in a["operators"])}

    osb, csb = next(p for p in a["pairs"] if p[0] == "[")
    ob, cb = next(p for p in a["pairs"] if p[0] == "{")
    repo["option_block"] = {
        "begin": re_escape(osb), "end": re_escape(csb),
        "beginCaptures": {"0": {"name": "punctuation.definition.attribute.begin.sig"}},
        "endCaptures": {"0": {"name": "punctuation.definition.attribute.end.sig"}},
        "patterns": [
            {"include": "#comment"},
            {"include": "#option_values"},
            {"match": r"(?<=" + re_escape(osb) + r"|,)\s*(" + ident + ")",
             "captures": {"1": {"name": "entity.other.attribute-name.sig"}}},
            {"include": "#values"},
        ],
    }
    # `{a, b}` inside an option is a value list, not more option names.
    repo["option_values"] = {"begin": re_escape(ob), "end": re_escape(cb), "patterns": [{"include": "#values"}]}

    values = [t for t in token_patterns if t not in ("insert_block", "comment")]
    repo["values"] = {"patterns": [{"include": "#" + p} for p in
                                   values + ["qualified"] + kw + ["function", "identifier", "operator"]]}

    # Keywords and built-in types before `function`, so `uint2(` stays a type.
    top = token_patterns + ["option_block", "declaration", "qualified"] + kw + ["function", "identifier", "operator"]

    return {
        "$schema": "https://raw.githubusercontent.com/martinring/tmlanguage/master/tmlanguage.json",
        "name": "SIG",
        "scopeName": SCOPE,
        "fileTypes": ["sig"],
        "comment": "GENERATED from sources/SIGParser/SIG.g4 by sources/SIGParser/editor/gen_vs_extension.py -- do not edit.",
        "patterns": [{"include": "#" + p} for p in top],
        "repository": repo,
    }


def build_language_configuration(rules, a):
    comment = begin_end(lexer_ast("COMMENT", rules))
    line_comment = literal_of(flatten(lexer_ast("COMMENT", rules))[1][0]) if not comment else None
    quotes = []
    for tok in ("STRING", "RAWEXPR"):
        if tok in rules:
            be = begin_end(lexer_ast(tok, rules))
            if be and be[0] == be[1]:
                quotes.append([be[0], be[1]])
    pairs = a["pairs"]
    # Auto-closing `%{` would race the `{` pair and produce `%{}}%`.
    single = [p for p in pairs if len(p[0]) == 1]
    return {
        "comments": {"lineComment": line_comment},
        "brackets": pairs,
        "autoClosingPairs": [{"open": o, "close": c} for o, c in single + quotes],
        "surroundingPairs": [[o, c] for o, c in single + quotes],
    }


def build_theme():
    from xml.sax.saxutils import escape
    entries = "".join(
        f"    <dict><key>scope</key><string>{escape(sel)}</string>"
        f"<key>settings</key><dict><key>vsclassificationtype</key><string>{escape(cls)}</string></dict></dict>\n"
        for sel, cls in THEME)
    return ('<?xml version="1.0" encoding="UTF-8"?>\n'
            '<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">\n'
            '<plist version="1.0"><dict><key>name</key><string>SIG Theme</string><key>settings</key><array>\n'
            + entries + '</array></dict></plist>\n')


# --- packaging ------------------------------------------------------------------

def vsix_files(version, grammar_json, langcfg_json, theme_xml):
    pkgdef = (
        "// Generated by gen_vs_extension.py\r\n"
        "[$RootKey$\\TextMate\\Repositories]\r\n"
        f"\"SIG\"=\"$PackageFolder$\\Grammars\"\r\n"
        "\r\n"
        "[$RootKey$\\TextMate\\LanguageConfiguration\\GrammarMapping]\r\n"
        f"\"{SCOPE}\"=\"$PackageFolder$\\language-configuration.json\"\r\n"
    )
    manifest = f"""<?xml version="1.0" encoding="utf-8"?>
<PackageManifest Version="2.0.0" xmlns="http://schemas.microsoft.com/developer/vsx-schema/2011">
  <Metadata>
    <Identity Id="{EXT_ID}" Version="{version}" Language="en-US" Publisher="Spectrum" />
    <DisplayName>{EXT_NAME}</DisplayName>
    <Description xml:space="preserve">Syntax highlighting for Spectrum .sig files, generated from SIG.g4.</Description>
  </Metadata>
  <Installation>
    <InstallationTarget Id="Microsoft.VisualStudio.Community" Version="[17.0, 19.0)">
      <ProductArchitecture>amd64</ProductArchitecture>
    </InstallationTarget>
  </Installation>
  <Assets>
    <Asset Type="Microsoft.VisualStudio.VsPackage" Path="sig.pkgdef" />
  </Assets>
  <Prerequisites>
    <Prerequisite Id="Microsoft.VisualStudio.Component.CoreEditor" Version="[17.0, 19.0)" DisplayName="Visual Studio core editor" />
  </Prerequisites>
</PackageManifest>
"""
    content_types = (
        '<?xml version="1.0" encoding="utf-8"?>'
        '<Types xmlns="http://schemas.openxmlformats.org/package/2006/content-types">'
        '<Default Extension="vsixmanifest" ContentType="text/xml" />'
        '<Default Extension="json" ContentType="application/json" />'
        '<Default Extension="pkgdef" ContentType="text/plain" />'
        '<Default Extension="tmTheme" ContentType="text/xml" />'
        '</Types>'
    )

    files = {
        "extension.vsixmanifest": manifest,
        "sig.pkgdef": pkgdef,
        "Grammars/sig.tmLanguage.json": grammar_json,
        # Named like the starter kit's cpp.tmLanguage.tmTheme, next to its grammar.
        "Grammars/sig.tmLanguage.tmTheme": theme_xml,
        "language-configuration.json": langcfg_json,
    }
    size = sum(len(v.encode("utf-8")) for v in files.values())
    ext_dir = "[installdir]\\Common7\\IDE\\Extensions\\SpectrumSIG"
    deps = {"Microsoft.VisualStudio.Component.CoreEditor": "[17.0,19.0)"}

    files["manifest.json"] = json.dumps({
        "id": EXT_ID, "version": version, "type": "Vsix", "vsixId": EXT_ID, "extensionDir": ext_dir,
        "files": [{"fileName": "/" + f, "sha256": None} for f in files],
        "installSizes": {"targetDrive": size}, "dependencies": deps,
    })
    files["catalog.json"] = json.dumps({
        "manifestVersion": "1.1",
        "info": {"id": f"{EXT_ID},version={version}", "manifestType": "Extension"},
        # VSIXInstaller's setup-engine path requires the Component entry
        # ("A value for 'Component' needs to be specified in the catalog").
        "packages": [{
            "id": "Component." + EXT_ID, "version": version, "type": "Component", "extension": True,
            "dependencies": {EXT_ID: version, **deps},
            "localizedResources": [{"language": "en-US", "title": EXT_NAME,
                                    "description": "Syntax highlighting for Spectrum .sig files."}],
        }, {
            "id": EXT_ID, "version": version, "type": "Vsix",
            "payloads": [{"fileName": "sig.vsix", "size": size}],
            "vsixId": EXT_ID, "extensionDir": ext_dir,
            "installSizes": {"targetDrive": size}, "dependencies": deps,
        }],
    })
    return files, content_types


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--install-user", action="store_true",
                    help="copy the grammar into %%USERPROFILE%%/.vs/Extensions/SIG (no VSIX needed)")
    args = ap.parse_args()

    with open(G4_PATH, encoding="utf-8") as f:
        rules = split_rules(f.read())

    a = analyse(rules)
    grammar = build_grammar(rules, a)
    langcfg = build_language_configuration(rules, a)

    grammar_json = json.dumps(grammar, indent=2)
    langcfg_json = json.dumps(langcfg, indent=2)

    # Monotonic, so a regenerated VSIX upgrades an installed one.
    version = "1." + time.strftime("%Y%m%d") + "." + str(int(time.strftime("%H")) * 60 + int(time.strftime("%M")))

    os.makedirs(OUT_DIR, exist_ok=True)
    theme_xml = build_theme()
    files, content_types = vsix_files(version, grammar_json, langcfg_json, theme_xml)

    vsix_path = os.path.join(OUT_DIR, "sig.vsix")
    with zipfile.ZipFile(vsix_path, "w", zipfile.ZIP_DEFLATED) as z:
        z.writestr("[Content_Types].xml", content_types)
        for name, data in files.items():
            z.writestr(name, data)

    # Folder layout for the documented no-VSIX route: <root>/Syntaxes/*.tmLanguage.json
    folder = os.path.join(OUT_DIR, "folder", "SIG")
    shutil.rmtree(folder, ignore_errors=True)
    os.makedirs(os.path.join(folder, "Syntaxes"))
    with open(os.path.join(folder, "Syntaxes", "sig.tmLanguage.json"), "w", encoding="utf-8") as f:
        f.write(grammar_json)
    with open(os.path.join(folder, "Syntaxes", "sig.tmLanguage.tmTheme"), "w", encoding="utf-8") as f:
        f.write(theme_xml)
    with open(os.path.join(folder, "language-configuration.json"), "w", encoding="utf-8") as f:
        f.write(langcfg_json)

    print(f"keywords: {len(a['decl'])} declaration, {sum(len(v) for v in a['groups'].values())} grouped, "
          f"{len(a['other'])} other; {len(a['operators'])} operators; brackets {a['pairs']}")
    print(f"wrote {vsix_path} (version {version})")

    if args.install_user:
        dest = os.path.join(os.path.expanduser("~"), ".vs", "Extensions", "SIG")
        shutil.rmtree(dest, ignore_errors=True)
        shutil.copytree(folder, dest)
        print(f"installed to {dest} -- restart Visual Studio")


if __name__ == "__main__":
    sys.exit(main())
