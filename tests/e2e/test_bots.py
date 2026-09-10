# -*- coding: utf-8 -*-
"""Otonom botlar: kadro dosyasıyla açılan sunucuda botların oyuna girmesi, 'kim'
listesinde görünmesi, kd'ye cevap vermesi, kendi başına hareket etmesi ve bot
adlarının oyuncular tarafından alınamaması (kendi sunucu örneğini başlatır)."""
import re
import time

import pytest

import mud
from conftest import spawn_server

PASSWORD = mud.PASSWORD
IMM_NAME = "Denemetanri"
BOT_REPLY_TIMEOUT = 45.0     # botlar ~saniyede bir düşünür; sohbet cevabı gecikebilir
QUIET_ROOM = 1203            # Valhalla (valhalla.are, yalnızca tanrılar): bot sohbeti duyulmaz


def ask(c, cmd, pattern, timeout=10.0):
    """Komutu gönder, beklenen deseni ve ardından istemi bekle.

    Botların asenkron çıktısı (kd, kdg, aynı odada 'söyle') her satırdan sonra istem
    getirdiğinden ``command()`` erken dönebilir; bu yüzden istemi değil deseni bekleriz."""
    c.send(cmd)
    scr = c.expect(pattern, timeout=timeout)
    rest = c.read_until_prompt(timeout=timeout)
    return mud.Screen(scr.raw + rest.raw, scr.matched, scr.match)   # desen eşleşmesi korunur


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
    srv = spawn_server("botlar", bots=True)
    # 'botlar' komutunu kullanabilecek seviye 100 bir gözlemci karakteri
    mud.write_player_file(srv, IMM_NAME, PASSWORD, level=100, trust=100, room=3001,
                          hmv=(500, 500, 500))
    srv.start(timeout=30.0)
    yield srv
    srv.stop()


@pytest.fixture
def imm(bot_server):
    """Gözlemci ölümsüzle açılmış bağlantı (sessiz odada); test sonunda 'ayrıl' + kapatma."""
    c = bot_server.connect()
    mud.login(c, IMM_NAME, PASSWORD)
    c.command("goto %d" % QUIET_ROOM)
    yield c
    c.quit()


def test_roster_is_loaded(bot_server):
    text = bot_server.log_text()
    m = re.search(r"\[bot\] (\d+) bot tanımı yüklendi", text)
    assert m is not None, "Bot kadrosu yüklenmedi.\n%s" % bot_server.log_tail()
    assert int(m.group(1)) >= 10


def test_bots_log_in_and_show_in_who(bot_server, imm, screens):
    names = wait_for_bot_logins(bot_server, 2)
    who = ask(imm, "kim", re.compile(r"Oyuncular: (\d+)"))
    screens.add("kim (botlarla)", who.text)
    plain = who.plain
    for name in names[:2]:
        assert name in plain, "%s 'kim' listesinde yok:\n%s" % (name, plain)
    assert int(who.match.group(1)) >= 3, plain
    # botlar 'kimdir' ve 'nerede' gibi listelerde de görünür
    whois = ask(imm, "kimdir %s" % names[0], re.compile(r"^\[ *\d+ .*\] .*%s" % names[0], re.M))
    assert names[0] in whois.plain


def test_bot_answers_a_tell(bot_server, imm, screens):
    names = wait_for_bot_logins(bot_server, 2)
    imm.command("kd %s selam" % names[0])
    scr = imm.expect(re.compile(r"%s kd:" % names[0]), timeout=BOT_REPLY_TIMEOUT)
    screens.add("botun kd cevabı", scr.text)


def test_kdg_channel_reaches_bots(bot_server, imm, screens):
    """kdg herkese açık konu dışı kanaldır; botlar duyar ve (kd ya da kdg ile) cevap verir."""
    names = wait_for_bot_logins(bot_server, 2)
    scr = ask(imm, "kdg", re.compile(r"KDG kanalı (açıldı|kapandı)"))
    if scr.match.group(1) == "kapandı":          # kanal kapalıysa aç
        ask(imm, "kdg", "KDG kanalı açıldı.")
    ask(imm, "kdg selam millet, kim var oyunda?", "[KDG] Sen:")
    any_bot = "|".join(names)
    scr = imm.expect(re.compile(r"\[KDG\] (%s)|(%s) kd:" % (any_bot, any_bot)), timeout=BOT_REPLY_TIMEOUT)
    screens.add("kdg cevabı", scr.text)


def test_bot_answers_say_in_character(bot_server, imm, screens):
    """söyle kanalı rol içidir: bot aynı odada söylenene diyarın diliyle, oyun dışı kısaltma
    ve surat kullanmadan 'söyle' ile cevap verir."""
    names = wait_for_bot_logins(bot_server, 2)
    imm.command("goto 3001")
    reply = None
    for name in names[:3]:
        imm.command("transfer %s" % name.lower())
        imm.command("söyle selam %s, nasılsın?" % name)
        try:
            scr = imm.expect(re.compile(r"%s '(.+?)' dedi\." % name), timeout=28.0)
        except mud.MudTimeout:
            continue
        reply = scr.match.group(1)
        screens.add("botun söyle cevabı", scr.text)
        break
    assert reply is not None, "Botlar 'söyle'ye cevap vermedi."
    assert not re.search(r":\)|:P|xd|\blvl\b|\beq\b|kanka|\bbb\b", reply), (
        "Rol içi kanalda oyun dışı ifade: %r" % reply)


def test_god_bot_answers_a_prayer(bot_server, imm, screens):
    """Kadrodaki tanrı botu ('!tanrı') dua eden sıkışmış ölümlüye kd ile cevap verip onu
    tapınağa alır (ölümsüzlerin duası dikkate alınmaz; bu yüzden bir ölümlü dua eder)."""
    wait_for_bot_logins(bot_server, 1)
    scr = ask(imm, "botlar bağla Ulgen", re.compile(r"Bot oyuna girdi|Zaten oyunda|Bot giremedi"))
    assert "giremedi" not in scr.plain, scr.plain
    who = ask(imm, "kim", re.compile(r"Oyuncular: \d+"))
    assert "Ulgen" in who.plain, who.plain
    time.sleep(0.6)
    mortal = bot_server.connect()
    try:
        mud.create_character(mortal, "Duaci", PASSWORD)
        mortal.command("dua Sıkıştım, çıkış yolu bulamıyorum!")
        # bot_god.c: kd cevabı 3-8 s, transfer 8-25 s sonra gelir; transfer sonrası oda görünür
        scr = mortal.expect(re.compile(r"Ulgen kd:"), timeout=BOT_REPLY_TIMEOUT)
        try:
            scr = scr + mortal.expect("Selenge Tapınağı", timeout=30.0)
        except mud.MudTimeout:
            pytest.fail("Tanrı botu ölümlüyü tapınağa almadı.\n%s" % scr.plain, pytrace=False)
        screens.add("tanrı botunun dua cevabı", scr.text)
    finally:
        mortal.close()


def test_bots_move_and_act_on_their_own(bot_server, imm, screens):
    wait_for_bot_logins(bot_server, 2)
    deadline = time.monotonic() + 60.0
    rows = []
    while time.monotonic() < deadline:
        scr = imm.command("botlar")
        rows = [l for l in scr.lines if re.match(r"\w+\s+\d+\s+\S+", l)]
        if any("Mud Okulu Girişi" not in l for l in rows):      # en az bir bot okuldan ayrıldı
            screens.add("botlar (hareket)", scr.text)
            break
        time.sleep(2.0)
    else:
        pytest.fail("Botlar 60 saniyede okul girişinden ayrılmadı:\n%s" % "\n".join(rows))
    assert bot_server.is_running()


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
