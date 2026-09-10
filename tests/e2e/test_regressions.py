# -*- coding: utf-8 -*-
"""Geçmişte düzeltilen sunucu hataları için bekçi testler (kendi sunucu örneğini başlatır)."""
import mud
from conftest import spawn_server


def test_say_survives_without_log_kanal_directory():
    dirs = [d for d in mud.MudServer.RUNTIME_DIRS if d not in ("log/kanal", "log/events")]
    srv = spawn_server("regression-kanal", runtime_dirs=dirs)
    srv.start(timeout=30.0)
    try:
        c = srv.connect()
        mud.create_character(c, "Denemekanal", mud.PASSWORD)
        c.send("söyle merhaba")
        scr = c.read_idle(idle=0.8, max_wait=5.0)
        assert srv.is_running(), "Sunucu 'söyle' komutunda çöktü.\n%s" % srv.log_tail()
        assert "merhaba" in scr.plain
        c.quit()
    finally:
        srv.stop()
