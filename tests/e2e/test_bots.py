# -*- coding: utf-8 -*-
"""Otonom botlar: kadro dosyasıyla açılan sunucuda botların oyuna girmesi, 'kim'
listesinde görünmesi, kd'ye cevap vermesi, kendi başına hareket etmesi ve bot
adlarının oyuncular tarafından alınamaması (kendi sunucu örneğini başlatır)."""
import datetime
import hashlib
import re
import time

import pytest

import mud
from conftest import RUNS_DIR, REPO, STATE, find_binary

PASSWORD = mud.PASSWORD
IMM_NAME = "Denemetanri"


def m1_hash(password, salt_hex="0123456789abcdef"):
    pw = password.encode("utf-8")
    digest = hashlib.sha256(salt_hex.encode("ascii") + pw).digest()
    for _ in range(10000):
        digest = hashlib.sha256(digest + pw).digest()
    return "$m1$%s$%s" % (salt_hex, digest.hex())


def seed_immortal(server, name, password):
    """'botlar' komutunu kullanabilecek seviye 100 bir gözlemci karakteri dosyası yaz."""
    content = (
        "#PLAYER\nName %s~\nPass %s~\nRace insan~\nSex  1\nCla  3\nLevl 100\nTrust 100\n"
        "Room 3001\nHMV  500 500 500\nExp  100000\nEnd\n\n#END\n" % (name, m1_hash(password))
    )
    server.player_file(name).write_bytes(content.encode("utf-8"))


def wait_for_bot_logins(server, count, timeout=75.0):
    """Sunucu günlüğünde en az ``count`` bot girişi görünene dek bekle; adları döndür."""
    deadline = time.monotonic() + timeout
    names = []
    while time.monotonic() < deadline:
        names = mud.BOT_LOGIN_RE.findall(server.log_text())
        if len(names) >= count:
            return names
        assert server.is_running(), "Sunucu botlar girerken sonlandı.\n%s" % server.log_tail()
        time.sleep(0.5)
    raise AssertionError("%.0f saniyede %d bot girmedi (girenler: %s).\n%s"
                         % (timeout, count, names, server.log_tail()))


@pytest.fixture(scope="module")
def bot_server():
    binary, error = find_binary()
    if binary is None:
        pytest.skip(error)
    stamp = datetime.datetime.now().strftime("%Y%m%d-%H%M%S-%f")
    base = STATE.run_dir if STATE.run_dir is not None else RUNS_DIR
    run_dir = base / ("botlar-" + stamp)
    srv = mud.MudServer(binary, REPO, run_dir).prepare(bots=True)
    seed_immortal(srv, IMM_NAME, PASSWORD)
    srv.start(timeout=30.0)
    yield srv
    srv.stop()


def test_roster_is_loaded(bot_server):
    text = bot_server.log_text()
    m = re.search(r"\[bot\] (\d+) bot tanımı yüklendi", text)
    assert m is not None, "Bot kadrosu yüklenmedi.\n%s" % bot_server.log_tail()
    assert int(m.group(1)) >= 10


def test_bots_log_in_and_show_in_who(bot_server, screens):
    names = wait_for_bot_logins(bot_server, 2)
    c = bot_server.connect()
    try:
        mud.login(c, IMM_NAME, PASSWORD)
        who = c.command("kim")
        screens.add("kim (botlarla)", who.text)
        plain = who.plain
        for name in names[:2]:
            assert name in plain, "%s 'kim' listesinde yok:\n%s" % (name, plain)
        m = re.search(r"Oyuncular: (\d+)", plain)
        assert m is not None and int(m.group(1)) >= 3
        # botlar 'kimdir' ve 'nerede' gibi listelerde de görünür
        whois = c.command("kimdir %s" % names[0])
        assert names[0] in whois.plain
    finally:
        c.quit()


def test_bot_answers_a_tell(bot_server, screens):
    names = wait_for_bot_logins(bot_server, 2)
    c = bot_server.connect()
    try:
        mud.login(c, IMM_NAME, PASSWORD)
        c.command("kd %s selam" % names[0])
        scr = c.read_idle(idle=12.0, max_wait=14.0)
        screens.add("botun kd cevabı", scr.text)
        assert re.search(r"%s kd:" % names[0], scr.plain), (
            "%s kd'ye cevap vermedi.\n%s" % (names[0], scr.plain))
    finally:
        c.quit()


def test_kdg_channel_reaches_bots(bot_server, screens):
    """kdg herkese açık konu dışı kanaldır; botlar duyar ve (kd ya da kdg ile) cevap verir."""
    names = wait_for_bot_logins(bot_server, 2)
    c = bot_server.connect()
    try:
        mud.login(c, IMM_NAME, PASSWORD)
        scr = c.command("kdg")
        if "kapandı" in scr.plain:          # kanal kapalıysa aç
            c.command("kdg")
        scr = c.command("kdg selam millet, kim var oyunda?")
        assert "[KDG] Sen:" in scr.plain, scr.plain
        scr = c.read_idle(idle=40.0, max_wait=45.0)
        screens.add("kdg cevabı", scr.text)
        assert re.search(r"(\[KDG\] (%s)|(%s) kd:)" % ("|".join(names), "|".join(names)), scr.plain), (
            "Hiçbir bot kdg'ye cevap vermedi.\n%s" % scr.plain)
    finally:
        c.quit()


def test_bot_answers_say_in_character(bot_server, screens):
    """söyle kanalı rol içidir: bot aynı odada söylenene diyarın diliyle, oyun dışı kısaltma
    ve surat kullanmadan 'söyle' ile cevap verir."""
    names = wait_for_bot_logins(bot_server, 2)
    c = bot_server.connect()
    try:
        mud.login(c, IMM_NAME, PASSWORD)
        c.command("goto 3001")
        reply = None
        for name in names[:3]:
            c.command("transfer %s" % name.lower())
            c.command("söyle selam %s, nasılsın?" % name)
            scr = c.read_idle(idle=25.0, max_wait=28.0)
            m = re.search(r"%s '(.+?)' dedi\." % name, scr.plain)
            if m:
                reply = m.group(1)
                screens.add("botun söyle cevabı", scr.text)
                break
        assert reply is not None, "Botlar 'söyle'ye cevap vermedi."
        assert not re.search(r":\)|:P|xd|\blvl\b|\beq\b|kanka|\bbb\b", reply), (
            "Rol içi kanalda oyun dışı ifade: %r" % reply)
    finally:
        c.quit()


def test_god_bot_answers_a_prayer(bot_server, screens):
    """Kadrodaki tanrı botu ('!tanrı') dua eden sıkışmış ölümlüye kd ile cevap verip onu
    tapınağa alır (ölümsüzlerin duası dikkate alınmaz; bu yüzden bir ölümlü dua eder)."""
    wait_for_bot_logins(bot_server, 1)
    imm = bot_server.connect()
    mortal = None
    try:
        mud.login(imm, IMM_NAME, PASSWORD)
        scr = imm.command("botlar bağla Ulgen")
        assert re.search(r"Bot oyuna girdi|Zaten oyunda", scr.plain), scr.plain
        who = imm.command("kim")
        assert "Ulgen" in who.plain, who.plain
        time.sleep(0.6)
        mortal = bot_server.connect()
        mud.create_character(mortal, "Duaci", PASSWORD)
        mortal.command("dua Sıkıştım, çıkış yolu bulamıyorum!")
        scr = mortal.read_idle(idle=40.0, max_wait=45.0)
        screens.add("tanrı botunun dua cevabı", scr.text)
        assert re.search(r"Ulgen kd:", scr.plain), "Tanrı botu duaya cevap vermedi.\n%s" % scr.plain
        assert "transferred you" in scr.plain or "Selenge Tapınağı" in scr.plain, scr.plain
    finally:
        if mortal is not None:
            mortal.close()
        imm.quit()


def test_bots_move_and_act_on_their_own(bot_server, screens):
    wait_for_bot_logins(bot_server, 2)
    c = bot_server.connect()
    try:
        mud.login(c, IMM_NAME, PASSWORD)
        deadline = time.monotonic() + 60.0
        rows = []
        while time.monotonic() < deadline:
            scr = c.command("botlar")
            rows = [l for l in scr.lines if re.match(r"\w+\s+\d+\s+\S+", l)]
            active = [l for l in rows if not re.search(r"\bboşta\b", l) or "Mud Okulu Girişi" not in l]
            if len(active) >= 1 and any("Mud Okulu Girişi" not in l for l in rows):
                screens.add("botlar (hareket)", scr.text)
                break
            time.sleep(2.0)
        else:
            pytest.fail("Botlar 60 saniyede okul girişinden ayrılmadı:\n%s" % "\n".join(rows))
        assert bot_server.is_running()
    finally:
        c.quit()


def test_bot_name_cannot_be_taken_by_a_player(bot_server):
    names = wait_for_bot_logins(bot_server, 1)
    c = bot_server.connect()
    try:
        c.wait_greeting()
        c.send(names[0])
        scr = c.expect(["Bu karakter oyunda", mud.PASSWORD_PROMPT, mud.CONFIRM_NEW_NAME], timeout=5.0)
        assert scr.matched == "Bu karakter oyunda", "Bot adı korunmadı:\n%s" % scr.plain
    finally:
        c.close()


def test_server_still_healthy_after_bots_played(bot_server):
    text = bot_server.log_text()
    assert bot_server.is_running(), "Sunucu botlarla çalışırken sonlandı.\n%s" % bot_server.log_tail()
    bad = [l for l in text.splitlines() if "Segmentation" in l or "Assertion" in l]
    assert not bad, "\n".join(bad)
