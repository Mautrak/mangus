# -*- coding: utf-8 -*-
"""ANSI SGR dizilerini (renk 30-37/90-97, arka plan, kalın, sıfırla) HTML'e çevirir
ve uçtan uca testlerin görsel raporunu (report.html) üretir. Yalnızca stdlib."""
import datetime
import html
import re

SGR_RE = re.compile(r"\x1b\[([0-9;]*)m")
OTHER_ESC_RE = re.compile(r"\x1b\[[0-9;?]*[ -/]*[@-ln-~]|\x1b[^\[]")

# 16 renklik palet (koyu zemin için ayarlı; 0 = siyah yerine koyu gri).
PALETTE = [
    "#3b3b3b", "#d54e4e", "#3fbf5f", "#d9b64a", "#5b7fe0", "#c05ec0", "#3fb8c8", "#d0d0d0",
    "#8a8a8a", "#ff6b6b", "#6dff8a", "#ffe66d", "#8fa8ff", "#ff8cff", "#7fefff", "#ffffff",
]

CSS = """
body { background: #0f1115; color: #d0d0d0; font-family: -apple-system, "Segoe UI", Helvetica, Arial, sans-serif; margin: 0; padding: 24px; }
h1, h2, h3 { color: #f0f0f0; font-weight: 600; }
a { color: #8fa8ff; }
pre.screen { background: #000; color: #d0d0d0; border: 1px solid #2a2d34; border-radius: 6px; padding: 12px; overflow-x: auto;
             font: 13px/1.35 "SF Mono", Menlo, Consolas, "DejaVu Sans Mono", monospace; white-space: pre; margin: 0; }
pre.diff { background: #14161b; border: 1px solid #2a2d34; border-radius: 6px; padding: 12px; overflow-x: auto;
           font: 12px/1.35 "SF Mono", Menlo, Consolas, monospace; white-space: pre; }
.diff .add { color: #6dff8a; } .diff .del { color: #ff6b6b; } .diff .hdr { color: #8fa8ff; } .diff .hunk { color: #ffe66d; }
table.summary { border-collapse: collapse; margin: 12px 0 24px; }
table.summary th, table.summary td { border: 1px solid #2a2d34; padding: 6px 12px; text-align: left; }
.badge { display: inline-block; padding: 2px 8px; border-radius: 10px; font-size: 12px; font-weight: 600; color: #000; }
.badge.passed { background: #6dff8a; } .badge.failed { background: #ff6b6b; } .badge.updated { background: #ffe66d; }
.badge.created { background: #7fefff; } .badge.missing { background: #ff8cff; } .badge.screen { background: #8a8a8a; }
.pair { display: flex; gap: 12px; flex-wrap: wrap; }
.pair > div { flex: 1 1 480px; min-width: 0; }
.pair h4 { margin: 8px 0 4px; color: #aaa; font-weight: 500; }
section { margin-bottom: 36px; }
.meta { color: #9a9a9a; font-size: 13px; }
details > summary { cursor: pointer; color: #8fa8ff; }
.b { font-weight: bold; }
"""
CSS += "".join(".f%d { color: %s; }\n" % (i, c) for i, c in enumerate(PALETTE))
CSS += "".join(".g%d { background: %s; }\n" % (i, c) for i, c in enumerate(PALETTE))


def ansi_to_html(text):
    """ANSI renkli metni HTML'e çevir (metin HTML olarak kaçırılır)."""
    text = OTHER_ESC_RE.sub("", text)
    out = []
    fg = bg = None
    bold = False
    pos = 0
    for m in SGR_RE.finditer(text):
        out.append(_span(text[pos:m.start()], fg, bg, bold))
        pos = m.end()
        params = [int(p) if p else 0 for p in m.group(1).split(";")] if m.group(1) else [0]
        for p in params:
            if p == 0:
                fg = bg = None
                bold = False
            elif p == 1:
                bold = True
            elif p == 22:
                bold = False
            elif 30 <= p <= 37:
                fg = p - 30
            elif p == 39:
                fg = None
            elif 90 <= p <= 97:
                fg = p - 90 + 8
            elif 40 <= p <= 47:
                bg = p - 40
            elif p == 49:
                bg = None
            elif 100 <= p <= 107:
                bg = p - 100 + 8
    out.append(_span(text[pos:], fg, bg, bold))
    return "".join(out)


def _span(chunk, fg, bg, bold):
    if not chunk:
        return ""
    escaped = html.escape(chunk, quote=False)
    classes = []
    if fg is not None:
        classes.append("f%d" % (fg + 8 if bold and fg < 8 else fg))
    if bg is not None:
        classes.append("g%d" % bg)
    if bold:
        classes.append("b")
    if not classes:
        return escaped
    return '<span class="%s">%s</span>' % (" ".join(classes), escaped)


def diff_to_html(diff_text):
    lines = []
    for line in diff_text.split("\n"):
        cls = ""
        if line.startswith("+++") or line.startswith("---"):
            cls = "hdr"
        elif line.startswith("@@"):
            cls = "hunk"
        elif line.startswith("+"):
            cls = "add"
        elif line.startswith("-"):
            cls = "del"
        esc = html.escape(line, quote=False)
        lines.append('<span class="%s">%s</span>' % (cls, esc) if cls else esc)
    return "\n".join(lines)


def render_report(screens, meta):
    """``screens``: ScreenRecord benzeri nesneler (name, nodeid, status, raw, expected, actual, diff).
    ``meta``: başlık bilgileri sözlüğü."""
    parts = []
    parts.append("<!DOCTYPE html>\n<html lang=\"tr\"><head><meta charset=\"utf-8\">")
    parts.append("<title>Mangus uçtan uca görsel rapor</title><style>%s</style></head><body>" % CSS)
    parts.append("<h1>Mangus uçtan uca / görsel regresyon raporu</h1>")
    parts.append("<p class=\"meta\">%s</p>" % " &middot; ".join(
        "%s: %s" % (html.escape(str(k)), html.escape(str(v)))
        for k, v in meta.items() if not str(k).startswith("_")))

    parts.append("<h2>Özet</h2><table class=\"summary\"><tr><th>Ekran</th><th>Durum</th><th>Test</th></tr>")
    for s in screens:
        parts.append("<tr><td><a href=\"#%s\">%s</a></td><td><span class=\"badge %s\">%s</span></td><td>%s</td></tr>" % (
            html.escape(s.anchor), html.escape(s.name), html.escape(s.status), html.escape(s.status),
            html.escape(s.nodeid)))
    parts.append("</table>")

    for s in screens:
        parts.append("<section id=\"%s\"><h2>%s <span class=\"badge %s\">%s</span></h2>" % (
            html.escape(s.anchor), html.escape(s.name), html.escape(s.status), html.escape(s.status)))
        parts.append("<p class=\"meta\">%s</p>" % html.escape(s.nodeid))
        if s.status in ("failed",) and s.expected is not None:
            parts.append("<div class=\"pair\"><div><h4>beklenen (altın dosya)</h4><pre class=\"screen\">%s</pre></div>"
                         "<div><h4>gerçekleşen</h4><pre class=\"screen\">%s</pre></div></div>" % (
                             ansi_to_html(s.expected), ansi_to_html(s.actual)))
            if s.diff:
                parts.append("<h4>fark</h4><pre class=\"diff\">%s</pre>" % diff_to_html(s.diff))
        else:
            parts.append("<pre class=\"screen\">%s</pre>" % ansi_to_html(s.raw))
        parts.append("</section>")

    log = meta.get("_log")
    if log:
        parts.append("<details><summary>Sunucu günlüğü</summary><pre class=\"diff\">%s</pre></details>"
                     % html.escape(log, quote=False))
    parts.append("<p class=\"meta\">Üretim: %s</p></body></html>" % datetime.datetime.now().isoformat(timespec="seconds"))
    return "\n".join(parts)
