#!/usr/bin/env python3
"""Generates workdir/shaders/materialx/mx_generated.hlsl.

Every MaterialX stdlib nodedef that is not hand-written in mx_stdlib.hlsl is
defined in MaterialX either by an inline expression ("sourcecode" in
genglsl/stdlib_genglsl_impl.mtlx) or by a nodegraph of other nodes
(stdlib_ng.mtlx). Both are mechanical to turn into HLSL, so they are generated
instead of hand-ported. Nodes that MaterialX implements with a .glsl file
(hsvtorgb, rotate, splits, ...) are hand-ported in mx_stdlib.hlsl.

Usage:  python gen_generated_hlsl.py [--libs <dir with stdlib/>] [--verbose]
        Defaults to workdir/materialx/libraries, so the output only depends on
        files committed to the repo, except for stdlib_ng.mtlx and the genglsl
        impl file, which are read from the vcpkg install of MaterialX.

Naming: a generated function is mx_<category>[_fa]_<outputtype>, the same
typed spelling MaterialXLibrary::register_nodes falls back to. Parameters are
named i_<input> because `in`/`out` are HLSL keywords and MaterialX uses both.
"""
import os
import re
import sys
import xml.etree.ElementTree as ET

REPO = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
VCPKG_LIBS = os.path.join(REPO, "vcpkg_installed", "x64-windows", "x64-windows", "libraries", "stdlib")
HLSL_DIR = os.path.join(REPO, "workdir", "shaders", "materialx")
HAND = os.path.join(HLSL_DIR, "mx_stdlib.hlsl")
OUT = os.path.join(HLSL_DIR, "mx_generated.hlsl")

TYPES = {"float": "float", "integer": "int", "boolean": "bool",
         "vector2": "float2", "vector3": "float3", "color3": "float3",
         "vector4": "float4", "color4": "float4"}
WIDTH = {"float": 1, "float2": 2, "float3": 3, "float4": 4}

verbose = "--verbose" in sys.argv


def literal(mx_type, value):
    h = TYPES[mx_type]
    value = (value or "").strip()
    if h == "int":
        return value or "0"
    if h == "bool":
        return "true" if value == "true" else "false"

    def f(s):
        s = s.strip() or "0.0"
        s = s if re.search(r"[.eEnN]", s) else s + ".0"
        # Suffixed so template deduction sees `float`, not a literal type.
        return s + "f"

    if h == "float":
        return f(value)
    comps = [f(c) for c in value.split(",")] if value else []
    comps += ["0.0"] * (WIDTH[h] - len(comps))
    return "%s(%s)" % (h, ", ".join(comps[:WIDTH[h]]))


class NodeDef:
    def __init__(self, el):
        self.name = el.get("name")
        self.category = el.get("node")
        self.inputs = [(i.get("name"), i.get("type"), i.get("value"), i.get("defaultgeomprop"))
                       for i in el.findall("input")]
        self.outputs = [(o.get("name"), o.get("type")) for o in el.findall("output")] or [("out", el.get("type"))]
        self.fa = self.name.endswith("FA")
        types = {t for _, t, _, _ in self.inputs} | {t for _, t in self.outputs}
        self.supported = types <= set(TYPES)
        base = "mx_" + self.category + ("_fa" if self.fa else "")
        self.generic = base
        self.typed = base + "_" + self.outputs[0][1]

    def signature(self):
        return tuple(TYPES[t] for _, t, _, _ in self.inputs)


def main():
    libs = VCPKG_LIBS
    for i, a in enumerate(sys.argv):
        if a == "--libs":
            libs = sys.argv[i + 1]

    defs_root = ET.parse(os.path.join(libs, "stdlib_defs.mtlx")).getroot()
    ng_root = ET.parse(os.path.join(libs, "stdlib_ng.mtlx")).getroot()
    impl_root = ET.parse(os.path.join(libs, "genglsl", "stdlib_genglsl_impl.mtlx")).getroot()

    nodedefs = {}
    by_category = {}
    for el in defs_root.iter("nodedef"):
        nd = NodeDef(el)
        nodedefs[nd.name] = nd
        by_category.setdefault(nd.category, []).append(nd)

    hand_src = open(HAND).read()
    hand = set(re.findall(r"(?<![A-Za-z0-9_])(mx_[A-Za-z0-9_]+)\s*\(", hand_src))

    sourcecode = {}
    glsl_file = set()
    for im in impl_root.iter("implementation"):
        nd = im.get("nodedef")
        if im.get("sourcecode") is not None:
            sourcecode[nd] = im.get("sourcecode")
        elif im.get("file"):
            glsl_file.add(nd)

    graphs = {g.get("nodedef"): g for g in ng_root.iter("nodegraph") if g.get("nodedef")}

    generated = set()   # typed function names emitted so far
    texts = []
    sigs = set()

    def resolve(nd):
        """HLSL function implementing nd, or None."""
        if nd.generic in hand:
            return nd.generic
        if nd.typed in hand or nd.typed in generated:
            return nd.typed
        return None

    def translate(expr):
        expr = expr.strip().rstrip(";").strip()
        expr = re.sub(r"\bfract\(", "frac(", expr)
        expr = re.sub(r"\bvec([234])\b", r"float\1", expr)
        expr = re.sub(r"\bmix\(", "lerp(", expr)
        expr = re.sub(r"\bmx_atan\(", "atan2(", expr)
        expr = re.sub(r"\bmx_(sin|cos|tan|asin|acos)\(", r"\1(", expr)
        expr = re.sub(r"\bmx_mod\(", "mx_modulo(", expr)
        return expr

    def header(nd, name):
        params = ["%s i_%s" % (TYPES[t], n) for n, t, _, _ in nd.inputs]
        if len(nd.outputs) == 1:
            return "%s %s(%s)" % (TYPES[nd.outputs[0][1]], name, ", ".join(params))
        params += ["out %s o_%s" % (TYPES[t], n) for n, t in nd.outputs]
        return "void %s(%s)" % (name, ", ".join(params))

    def gen_sourcecode(nd):
        if len(nd.outputs) != 1:
            return None
        expr = translate(sourcecode[nd.name])
        expr = re.sub(r"\{\{(\w+)\}\}", lambda m: "i_" + m.group(1), expr)
        return "%s\n{\n    return %s;\n}\n" % (header(nd, nd.typed), expr)

    def resolve_node(el, ng_nodes):
        """Pick the nodedef an internal nodegraph node instantiates."""
        outs = [o.get("type") for o in el.findall("output")]
        node_type = el.get("type")
        provided = {i.get("name"): i.get("type") for i in el.findall("input")}
        for nd in by_category.get(el.tag, []):
            if not nd.supported:
                continue
            if node_type != "multioutput" and nd.outputs[0][1] != node_type:
                continue
            have = {n: t for n, t, _, _ in nd.inputs}
            if all(have.get(n) == t for n, t in provided.items()):
                return nd
        return None

    def gen_graph(nd):
        g = graphs[nd.name]
        nodes = {c.get("name"): c for c in g if c.tag not in ("output", "input")}
        order, state = [], {}

        def visit(name):
            if state.get(name) == 2:
                return True
            if state.get(name) == 1:
                return False
            if name not in nodes:
                return False
            state[name] = 1
            for i in nodes[name].findall("input"):
                if i.get("nodename") and not visit(i.get("nodename")):
                    return False
            state[name] = 2
            order.append(name)
            return True

        outputs = {o.get("name"): o for o in g.findall("output")}
        for oname, _ in nd.outputs:
            o = outputs.get(oname)
            if o is None:
                return None
            if o.get("nodename") and not visit(o.get("nodename")):
                return None

        lines = []
        for name in order:
            el = nodes[name]
            inner = resolve_node(el, nodes)
            if inner is None:
                return None
            fn = resolve(inner)
            if fn is None:
                return None
            args = []
            provided = {i.get("name"): i for i in el.findall("input")}
            for n, t, v, _ in inner.inputs:
                i = provided.get(n)
                if i is None:
                    if v is None:   # geometry-defaulted input: no constant to fall back on
                        return None
                    args.append(literal(t, v))
                elif i.get("interfacename"):
                    args.append("i_" + i.get("interfacename"))
                elif i.get("nodename"):
                    ref = i.get("nodename")
                    ref_def = resolve_node(nodes[ref], nodes)
                    if ref_def is None:
                        return None
                    out = i.get("output")
                    args.append("n_%s%s" % (ref, "_" + out if out else ""))
                else:
                    args.append(literal(t, i.get("value")))
            if len(inner.outputs) == 1:
                lines.append("    %s n_%s = %s(%s);" % (TYPES[inner.outputs[0][1]], name, fn, ", ".join(args)))
            else:
                outs = []
                for on, ot in inner.outputs:
                    lines.append("    %s n_%s_%s;" % (TYPES[ot], name, on))
                    outs.append("n_%s_%s" % (name, on))
                lines.append("    %s(%s);" % (fn, ", ".join(args + outs)))

        body = []
        results = []
        for oname, otype in nd.outputs:
            o = outputs[oname]
            if o.get("interfacename"):
                results.append("i_" + o.get("interfacename"))
            elif o.get("nodename"):
                out = o.get("output")
                results.append("n_%s%s" % (o.get("nodename"), "_" + out if out else ""))
            else:
                results.append(literal(otype, o.get("value")))

        if len(nd.outputs) == 1:
            body = lines + ["    return %s;" % results[0]]
        else:
            body = lines + ["    o_%s = %s;" % (n, r) for (n, _), r in zip(nd.outputs, results)]
        return "%s\n{\n%s\n}\n" % (header(nd, nd.typed), "\n".join(body))

    pending = [nd for nd in nodedefs.values()
               if nd.supported and nd.generic not in hand and nd.typed not in hand]
    progress = True
    while progress:
        progress = False
        for nd in list(pending):
            text = None
            if nd.name in sourcecode:
                text = gen_sourcecode(nd)
            elif nd.name in graphs:
                text = gen_graph(nd)

            if text is None:
                continue

            sig = (nd.typed, nd.signature())
            pending.remove(nd)

            if sig in sigs:
                continue

            sigs.add(sig)
            generated.add(nd.typed)
            texts.append((nd, text))
            progress = True

    out = ["// Generated by workdir/materialx/gen_generated_hlsl.py -- do not edit.",
           "// MaterialX stdlib nodedefs that mx_stdlib.hlsl does not hand-write:",
           "// inline-expression and nodegraph implementations, in dependency order.",
           "#ifndef MX_GENERATED_HLSL", "#define MX_GENERATED_HLSL", ""]

    for nd, text in texts:
        out.append("// " + nd.name)
        out.append(text)

    out.append("#endif")

    with open(OUT, "w", newline="\n") as f:
        f.write("\n".join(out) + "\n")

    print("generated %d functions; unresolved nodedefs: %d" % (len(texts), len(pending)))
    if verbose:
        for nd in pending:
            why = "glsl file" if nd.name in glsl_file else ("nodegraph" if nd.name in graphs else "no impl")
            print("  unresolved", nd.name, "(%s)" % why)


if __name__ == "__main__":
    main()
