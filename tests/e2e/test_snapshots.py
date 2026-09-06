# -*- coding: utf-8 -*-
"""Görsel regresyon: sabit bir karakter (insan / savaşçı) için ekranlar yakalanır ve
tests/e2e/snapshots/<ad>.txt altın dosyalarıyla (ANSI renk kodları dahil) karşılaştırılır.

Normalleştirme (karşılaştırmadan önce):
* telnet baytları ayıklanır, "\\n\\r" -> "\\n", satır sonu boşlukları atılır;
* komut istemi satırı (^Yp:.*<.*>) ve Discord uyarısı atılır;
* yalnızca dinamik sayı içeren ekranlarda (skor, kim) rakam dizileri "#" olur;
* envanter: acemi eşyalarının rastgele üretilen malzeme/durum etiketleri, yıldız
  bloğu ve rastgele silah adı yer tutucuya çevrilir (bkz. src/db.c create_object).

Altın dosyaları yenilemek için: pytest tests/e2e --update-snapshots
Dosya adları ASCII'ye çevrilir (çıkışlar -> cikislar.txt) ki her platformda güvenli olsun.
"""
import re

import pytest

import mud

CHARACTER = "Gorsel"                      # sabit ad: skor ekranında görünür
SCREENS = ["greeting", "bak", "skor", "yetenekler", "komutlar", "çıkışlar",
           "envanter", "ekipman", "yardım"]
DIGIT_SCREENS = {"skor", "kim"}           # dinamik sayılar içerenler

PROMPT_LINE_RE = re.compile(r"^Yp:.*<.*>")
DISCORD_MARK = "Discord ID'niz"
ESC = r"\x1b\[[0-9;]*m"
# "[ESC stone ESC]" gibi renkli malzeme/durum etiketleri -> "[…]" (renk de rastgele olabilir)
TAG_RE = re.compile(r"\[(?:%s)+[^\x1b\[\]]*(?:%s)+\]" % (ESC, ESC))
# durum yıldızları: "[ESC * ESC * ESC * ESC ]"
STARS_RE = re.compile(r"\[(?:%s|[* ])+\]" % ESC)
# Acemi silahının adı rastgele seçilir (src/obj_creator.c, obj_random_* / ITEM_WEAPON):
WEAPON_NAMES = ["egzotik bir silah", "bilinmeyen bir silah", "bir kılıç", "bir hançer", "bir mızrak",
                "bir topuz", "bir balta", "bir döven", "bir kırbaç", "bir teber", "bir yay", "bir ok",
                "bir kargı"]
WEAPON_RE = re.compile("(?:%s)(?=\x1b)" % "|".join(re.escape(n) for n in WEAPON_NAMES))
# Rastgele ek bayraklar (obj_random_extra_flag, eşya başına %1): ITEM_GLOW/ITEM_HUM eşya adının
# önüne renkli "[parlayan] " / "[vızıldayan] " ekler (act_info.c format_obj_to_char).
FLAG_PREFIX_RE = re.compile(r"(?:%s)?\[(?:parlayan|vızıldayan|görünmez|kızıl aura|mavi aura|büyülü)\] " % ESC)


def _mask_number(m):
    # sabit genişlikli alanlarda dolgu boşlukları rakam sayısına göre değişir;
    # sayının iki yanındaki boşluk dizilerini en çok bir boşluğa indir.
    return ("" if not m.group(1) else " ") + "#" + ("" if not m.group(2) else " ")


def mask_digits(line):
    """Rakam dizilerini '#' yap; ANSI kaçış dizilerinin içine (renk numaraları) dokunma."""
    parts = mud.ANSI_RE.split(line)
    codes = mud.ANSI_RE.findall(line)
    out = []
    for i, part in enumerate(parts):
        out.append(re.sub(r"( *)\d+( *)", _mask_number, part))
        if i < len(codes):
            out.append(codes[i])
    return "".join(out)


def normalise(name, text):
    text = mud.normalise_newlines(text)
    out = []
    for line in text.split("\n"):
        plain = mud.strip_ansi(line)
        if PROMPT_LINE_RE.match(plain) or DISCORD_MARK in plain:
            continue
        if name in DIGIT_SCREENS:
            line = mask_digits(line)
        if name == "envanter":
            line = STARS_RE.sub("[***]", line)
            line = TAG_RE.sub("[…]", line)
            line = WEAPON_RE.sub("<silah>", line)
            line = FLAG_PREFIX_RE.sub("", line)
        out.append(line.rstrip())
    return "\n".join(out).rstrip("\n") + "\n"


NEWBIE_ITEMS = 8          # acemi eşyaları: kalkan, silah, yelek, bayrak, 4 harita
ITEM_LINE_RE = re.compile(r"^\s+\[[* ]*\]")


def inventory_is_clean(text):
    """ITEM_INVIS bayrağı (obj_random_extra_flag, eşya başına %1) bir acemi eşyasını
    listeden gizler; eksik eşya varsa karakter yeniden yaratılır."""
    items = [line for line in mud.normalise_newlines(mud.strip_ansi(text)).split("\n")
             if ITEM_LINE_RE.match(line)]
    return len(items) == NEWBIE_ITEMS


@pytest.fixture(scope="module")
def captured(gate):
    """Tüm ekranları tek bağlantıda, sabit sırayla yakala (ham metin, ANSI dahil).

    Seviye 1 karakterler diske yazılmadığı için aynı ad yeniden yaratılabilir."""
    last = None
    for _attempt in range(5):
        c = gate.connect()
        shots = {"greeting": c.wait_greeting(timeout=10.0).text}
        mud.create_character(c, CHARACTER, mud.PASSWORD)
        for cmd in SCREENS[1:]:
            shots[cmd] = c.command(cmd, timeout=10.0).text
        gate.release(c)                   # 'ayrıl' + kapat
        if inventory_is_clean(shots["envanter"]):
            return shots
        last = shots["envanter"]
    pytest.fail("Beş denemede de acemi envanteri deterministik çıkmadı:\n%s" % mud.strip_ansi(last))


@pytest.mark.parametrize("name", SCREENS)
def test_screen_matches_snapshot(name, captured, snapshot_store, request):
    raw = captured[name]
    actual = normalise(name, raw)
    assert actual.strip(), "Boş ekran: %s" % name
    status, message = snapshot_store.check(name, actual, raw_text=raw, nodeid=request.node.nodeid)
    if status in ("failed", "missing"):
        pytest.fail(message, pytrace=False)
