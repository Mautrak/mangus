# -*- coding: utf-8 -*-
"""Giriş akışları: karakter oluşturma, ayrılma, yeniden giriş, yanlış parola, geçersiz adlar,
kayıtlı karakter dosyası ve $m1$ parola özeti."""
import hashlib
import os
import re
from pathlib import Path

import pytest

import mud

FIXTURES = Path(__file__).resolve().parent / "fixtures"
FIXTURE_NAME = "Denemeuc"        # fixtures/Denemeuc ve Denemeuc.legacy: seviye 2, parola sifre123

PASSWORD = mud.PASSWORD
PASSWORD_REPROMPT = "Şifre: "


def m1_hash(password, salt_hex):
    """src/password.c ile aynı türetme: SHA256(tuz_hex || parola), sonra 10000 tur
    SHA256(özet || parola); sonuç '$m1$<tuz>$<özet>'."""
    pw = password.encode("utf-8")
    digest = hashlib.sha256(salt_hex.encode("ascii") + pw).digest()
    for _ in range(10000):
        digest = hashlib.sha256(digest + pw).digest()
    return "$m1$%s$%s" % (salt_hex, digest.hex())


def seed_player_file(server, name, password, level=2, salt_hex="0123456789abcdef"):
    """Sunucunun okuyabildiği en küçük oyuncu dosyasını yaz (seviye >= 2 ki 'ayrıl'
    dosyayı yeniden yazsın; save.c seviye 1 karakterleri kaydetmez)."""
    content = (
        "#PLAYER\n"
        "Name %s~\n"
        "Pass %s~\n"
        "Race insan~\n"
        "Sex  1\n"
        "Cla  3\n"
        "Levl %d\n"
        "Room 3700\n"
        "HMV  20 100 100\n"
        "Exp  %d\n"
        "End\n\n"
        "#END\n" % (name, m1_hash(password, salt_hex), level, level * 1000)
    )
    path = server.player_file(name)
    path.write_bytes(content.encode("utf-8"))
    return path


def wait_for_disconnect(c, timeout=5.0):
    c.read_idle(idle=0.5, max_wait=timeout)
    return c.closed_by_server


def test_create_character_reaches_game(client, new_name, screens):
    name = new_name()
    c = client()
    transcript = mud.create_character(c, name, PASSWORD)
    screens.add("karakter oluşturma (%s)" % name, transcript)
    assert "Tebrikler! Karakterin savaşçı sınıfından." in transcript
    assert "Tanrılar sana bazı eşyalar bahşediyor." in transcript
    assert "Mud Okulu Girişi" in transcript, "Başlangıç odası görünmedi"
    assert mud.PROMPT_RE.search(transcript), "Oyun istemi (Yp:...) görünmedi"
    look = c.command("bak")
    assert "Mud Okulu Girişi" in look.plain


def test_quit_closes_connection_and_level_one_is_not_saved(client, new_name, server):
    name = new_name()
    c = client()
    mud.create_character(c, name, PASSWORD)
    c.send("ayrıl")
    c.expect(mud.QUIT_MESSAGE, timeout=5.0)
    c.in_game = False
    assert wait_for_disconnect(c), "'ayrıl' sonrasında sunucu bağlantıyı kapatmadı"
    assert "%s oyundan ayrıldı." % name in server.log_text()
    # save.c: seviyesi 2'den küçük karakterler diske yazılmaz; yeni karakterin dosyası olmaz.
    assert not server.player_file(name).exists(), (
        "Seviye 1 karakter için oyuncu dosyası yazıldı: %s (save_char_obj kuralı değişti mi?)"
        % server.player_file(name))


def test_relogin_to_linkdead_character_with_correct_password(client, new_name):
    name = new_name()
    c = client()
    mud.create_character(c, name, PASSWORD)
    c.close()                       # bağlantı kopar, karakter oyunda bağlantısız kalır
    c2 = client()
    transcript = mud.login(c2, name, PASSWORD)
    assert "Tekrar bağlanıyor" in transcript, "Yeniden bağlanma iletisi yok"
    assert mud.PROMPT_RE.search(transcript)
    assert name in c2.command("skor").plain


def test_wrong_password_is_rejected_and_third_attempt_disconnects(client, new_name):
    name = new_name()
    c = client()
    mud.create_character(c, name, PASSWORD)
    c.close()
    c2 = client()
    c2.wait_greeting()
    c2.send(name)
    c2.expect(mud.PASSWORD_PROMPT)
    for attempt in (1, 2):
        c2.send("yanlis%d" % attempt)
        scr = c2.expect(PASSWORD_REPROMPT)
        assert mud.WRONG_PASSWORD in scr.text, "Deneme %d: 'Yanlış şifre.' görünmedi" % attempt
    c2.send("yanlis3")
    c2.expect(mud.WRONG_PASSWORD)
    assert wait_for_disconnect(c2), "Üçüncü yanlış parolada bağlantı kapanmalı"
    # temizlik ve doğrulama: doğru parola hâlâ geçerli
    c3 = client()
    assert "Tekrar bağlanıyor" in mud.login(c3, name, PASSWORD)


def test_invalid_names_are_rejected(client):
    c = client()
    c.wait_greeting()
    for bad in ("x", "abc123", "Testoyuncu1", "Abcdefghijklm", "çağrı"):
        c.send(bad)
        scr = c.expect("İsim: ")
        assert mud.BAD_NAME in scr.text, "%r kabul edilmemeliydi: %r" % (bad, scr.text)
    assert not c.closed_by_server


def test_saved_character_login_and_quit_rewrites_m1_password_hash(client, server, new_name):
    name = new_name()
    path = seed_player_file(server, name, PASSWORD, level=2)
    c = client()
    c.wait_greeting()
    c.send(name)
    c.expect(mud.PASSWORD_PROMPT)
    c.send("yanlis")
    scr = c.expect(PASSWORD_REPROMPT)
    assert mud.WRONG_PASSWORD in scr.text
    c.send(PASSWORD)
    scr = c.expect(mud.PAGER, timeout=10.0)          # MOTD
    assert "Mangus oyuncu katline izin veren" in scr.plain
    c.send("")
    entered = c.read_until_prompt(timeout=10.0)
    c.in_game, c.name = True, name
    assert "Mud Okulu Girişi" in entered.plain
    assert name in c.command("skor").plain
    c.send("ayrıl")
    c.expect(mud.QUIT_MESSAGE)
    c.in_game = False
    assert wait_for_disconnect(c)

    content = path.read_bytes().decode("utf-8")
    lines = content.splitlines()
    assert any(line.startswith("LogO ") for line in lines), "Sunucu dosyayı yeniden yazmadı:\n%s" % content
    assert "Levl 2" in lines
    pass_lines = [line for line in lines if line.startswith("Pass ")]
    assert pass_lines, "Pass satırı yok:\n%s" % content
    m = re.match(r"Pass \$m1\$([0-9a-f]{16})\$([0-9a-f]{64})~$", pass_lines[0])
    assert m, "Pass satırı $m1$ biçiminde değil: %r" % pass_lines[0]
    assert m1_hash(PASSWORD, m.group(1)) == "$m1$%s$%s" % (m.group(1), m.group(2))


def install_fixture(server, fixture):
    """fixtures/<fixture> oyuncu dosyasını çalışma dizinine kopyala (bayt bayt)."""
    src = FIXTURES / fixture
    dst = server.player_file(FIXTURE_NAME)
    dst.write_bytes(src.read_bytes())
    return dst


def quit_and_wait(c):
    c.send("ayrıl")
    c.expect(mud.QUIT_MESSAGE)
    c.in_game = False
    assert wait_for_disconnect(c)


def test_fixture_player_file_logs_in_and_round_trips_m1_hash(client, server):
    path = install_fixture(server, FIXTURE_NAME)
    fixture_pass = [l for l in (FIXTURES / FIXTURE_NAME).read_text(encoding="utf-8").splitlines()
                    if l.startswith("Pass ")][0]
    c = client()
    transcript = mud.login(c, FIXTURE_NAME, PASSWORD)
    assert "Mud Okulu Girişi" in transcript
    assert "Denemeuc Acemi" in c.command("skor").plain
    quit_and_wait(c)
    lines = path.read_bytes().decode("utf-8").splitlines()       # katı: dosya UTF-8
    assert any(l.startswith("LogO ") for l in lines), "dosya yeniden yazılmadı"
    assert fixture_pass in lines, "$m1$ özeti değişmeden korunmalıydı"


@pytest.mark.skipif(os.name == "nt", reason="Eski crypt(3) özetleri Windows'ta doğrulanamaz (password.c)")
def test_legacy_player_file_login_upgrades_hash_and_latin5_title(client, server):
    """fixtures/Denemeuc.legacy: parola eski DES özeti (crypt), ünvan ISO-8859-9.
    Girişte parola kabul edilmeli; kayıtta özet $m1$'e yükseltilmeli ve dosya UTF-8 olmalı."""
    path = install_fixture(server, FIXTURE_NAME + ".legacy")
    c = client()
    transcript = mud.login(c, FIXTURE_NAME, PASSWORD)
    assert "Mud Okulu Girişi" in transcript
    skor = c.command("skor")
    skor.raw.decode("utf-8", "strict")
    assert "Işıklı Şövalye" in skor.plain, "Latin-5 ünvan UTF-8'e çevrilmemiş: %r" % skor.plain[:300]
    quit_and_wait(c)
    lines = path.read_bytes().decode("utf-8").splitlines()       # katı: artık UTF-8
    pass_line = [l for l in lines if l.startswith("Pass ")][0]
    m = re.match(r"Pass \$m1\$([0-9a-f]{16})\$([0-9a-f]{64})~$", pass_line)
    assert m, "Eski özet $m1$ biçimine yükseltilmedi: %r" % pass_line
    assert m1_hash(PASSWORD, m.group(1)) == "$m1$%s$%s" % (m.group(1), m.group(2))
    assert any("Işıklı Şövalye" in l for l in lines if l.startswith("Titl"))
