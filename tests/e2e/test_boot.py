# -*- coding: utf-8 -*-
"""Sunucu açılışı: hazır satırı, ölümcül hata yok, port bağlanıyor, karşılama UTF-8."""
import re

import mud

# Açılışta beklenen "[*****] BUG: Fix_exits:" satırları (bozuk çıkışlar) sorun değildir;
# aşağıdakiler ise gerçek hatadır.
FATAL_RE = re.compile(r"MAX_STRING|exceeded|Fread_string: EOF")


def test_server_boots_and_reports_ready(server):
    log = server.log_text()
    assert mud.READY_MARKER in log, "Hazır satırı yok.\n--- günlük ---\n%s" % server.log_tail()
    assert server.is_running(), "Sunucu süreci sonlanmış.\n--- günlük ---\n%s" % server.log_tail()


def test_boot_log_has_no_fatal_messages(server):
    bad = [line for line in server.log_text().splitlines() if FATAL_RE.search(line)]
    assert not bad, "Açılış günlüğünde ölümcül iletiler var:\n%s" % "\n".join(bad)


def test_port_accepts_connection_and_greets_in_utf8(client, screens):
    c = client()
    greeting = c.wait_greeting(timeout=10.0)
    # Katı çözümleme: sunucu çıktısı geçerli UTF-8 olmalı (Latin-5 kaçağı yok).
    text = greeting.raw.decode("utf-8", "strict")
    assert mud.NAME_PROMPT in text
    assert "Uzak Diyarlar tabanlı Türkçe MUD." in greeting.plain
    assert text.endswith(mud.NAME_PROMPT), "Karşılama isim istemiyle bitmeli"
    assert not c.closed_by_server
    screens.add("karşılama", greeting.text)
