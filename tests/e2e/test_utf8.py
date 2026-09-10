# -*- coding: utf-8 -*-
"""UTF-8 girdi/çıktı ve Latin-5 (ISO-8859-9) girdi uyumluluğu."""
import pytest

import mud

NAME = "Denemeutf"
TURKISH = "çğıöşüÇĞİÖŞÜ"


@pytest.fixture(scope="module")
def player(gate):
    """Modül boyunca oyunda kalan tek karakter. Sunucu aynı anda tek bağlantıya izin
    verdiğinden bu modülde ``client`` fikstürü kullanılmaz (bağlantıyı kapatırdı)."""
    c = gate.connect()
    c.creation_transcript = mud.create_character(c, NAME, mud.PASSWORD)
    yield c
    gate.release(c)


def test_class_name_typed_in_utf8_is_accepted(player):
    assert "Tebrikler! Karakterin savaşçı sınıfından." in player.creation_transcript


def test_help_command_typed_in_utf8(player, screens):
    scr = player.command("yardım")
    scr.raw.decode("utf-8", "strict")          # katı: geçersiz bayt yok
    plain = scr.plain
    assert "HAREKET" in plain and "yardım <komut>" in plain
    assert any(ch in plain for ch in TURKISH), "Yanıtta Türkçe harf yok"
    screens.add("yardım (UTF-8 girdi)", scr.text)


def test_latin5_input_gives_same_reply_as_utf8(player):
    utf8_reply = player.command("yardım")
    player.send_raw(b"yard\xfdm\r\n")          # ISO-8859-9: 0xFD = 'ı'
    latin5_reply = player.read_until_prompt(timeout=10.0)
    assert mud.strip_prompt_lines(latin5_reply.text) == mud.strip_prompt_lines(utf8_reply.text)


def test_turkish_text_round_trips_through_say(player):
    phrase = "Işıklı şöminede çığlık ğ ü ö"
    scr = player.command("söyle " + phrase)
    scr.raw.decode("utf-8", "strict")
    assert phrase in scr.plain, "Söylenen metin aynen dönmedi: %r" % scr.plain


def test_unknown_command_answers_hi(player):
    scr = player.command("xyzzy")
    assert scr.plain.lstrip().startswith("Hı?"), repr(scr.plain)
