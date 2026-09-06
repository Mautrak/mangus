# -*- coding: utf-8 -*-
"""Uçtan uca testler için pytest yapılandırması.

* ``server``  : oturum boyunca tek bir Mangus süreci (MANGUS_BIN).
* ``gate``    : aynı anda tek bağlantı; kapanışlar arasında 0.5 s boşluk.
* ``client``  : test başına bağlantı fabrikası (otomatik 'ayrıl' + kapatma).
* ``--update-snapshots`` / ``UPDATE_SNAPSHOTS=1`` : altın dosyaları yenile.
* Oturum sonunda ``tests/e2e/_runs/report.html`` görsel raporu yazılır.
"""
import datetime
import difflib
import itertools
import os
import shutil
import sys
import time
from pathlib import Path

import pytest

HERE = Path(__file__).resolve().parent
REPO = HERE.parent.parent
RUNS_DIR = HERE / "_runs"
SNAPSHOT_DIR = HERE / "snapshots"
if str(HERE) not in sys.path:
    sys.path.insert(0, str(HERE))

import mud  # noqa: E402
import ansi2html  # noqa: E402

CONNECTION_GAP = 0.5      # aynı makineden ardışık bağlantılar arasında bekleme (s)
KEEP_RUNS = 5             # _runs/ altında saklanacak eski çalışma dizini sayısı
PASSWORD = "sifre123"     # testlerin kullandığı ortak parola (>= 5 karakter)


# --------------------------------------------------------------------------
# seçenekler
# --------------------------------------------------------------------------
def pytest_addoption(parser):
    parser.addoption(
        "--update-snapshots", action="store_true", default=False,
        help="Görsel regresyon altın dosyalarını (tests/e2e/snapshots) yeniden yaz.")


def update_snapshots_enabled(config):
    if config.getoption("--update-snapshots"):
        return True
    return os.environ.get("UPDATE_SNAPSHOTS", "").strip().lower() not in ("", "0", "false", "no", "hayır")


def find_binary():
    """MANGUS_BIN, sonra <repo>/build/mangus[.exe], sonra <repo>/src/mangus[.exe]."""
    exe = ".exe" if os.name == "nt" else ""
    env = os.environ.get("MANGUS_BIN", "").strip()
    if env:
        p = Path(env)
        candidates = [p] if p.is_absolute() else [Path.cwd() / p, REPO / p]
        for c in candidates:
            if c.is_file():
                return c.resolve(), None
        return None, "MANGUS_BIN=%r bulunamadı (denenen: %s)" % (env, ", ".join(str(c) for c in candidates))
    for c in (REPO / "build" / ("mangus" + exe), REPO / "src" / ("mangus" + exe)):
        if c.is_file():
            return c.resolve(), None
    return None, ("Sunucu ikilisi bulunamadı; MANGUS_BIN ortam değişkenini ayarlayın "
                  "ya da build/mangus%s derleyin." % exe)


# --------------------------------------------------------------------------
# oturum durumu (rapor için)
# --------------------------------------------------------------------------
class ScreenRecord(object):
    def __init__(self, name, nodeid, raw, status="screen", expected=None, actual=None, diff=""):
        self.name = name
        self.nodeid = nodeid
        self.raw = raw
        self.status = status
        self.expected = expected
        self.actual = actual
        self.diff = diff

    @property
    def anchor(self):
        base = "".join(ch if ch.isalnum() else "-" for ch in (self.nodeid + "-" + self.name))
        return "s-%s-%d" % (base, id(self) % 100000)


class _State(object):
    server = None
    run_dir = None
    binary = None
    screens = []
    last_close = 0.0


STATE = _State()


def _prune_runs(keep):
    if not RUNS_DIR.exists():
        return
    dirs = sorted((p for p in RUNS_DIR.iterdir() if p.is_dir()), key=lambda p: p.stat().st_mtime)
    for old in dirs[:-keep] if keep else dirs:
        shutil.rmtree(old, ignore_errors=True)


# --------------------------------------------------------------------------
# sunucu
# --------------------------------------------------------------------------
@pytest.fixture(scope="session")
def server():
    binary, error = find_binary()
    if binary is None:
        if os.environ.get("MANGUS_BIN"):
            pytest.fail(error, pytrace=False)
        pytest.skip(error)
    RUNS_DIR.mkdir(parents=True, exist_ok=True)
    _prune_runs(KEEP_RUNS - 1)
    run_dir = RUNS_DIR / datetime.datetime.now().strftime("%Y%m%d-%H%M%S-%f")
    srv = mud.MudServer(binary, REPO, run_dir).prepare()
    STATE.server, STATE.run_dir, STATE.binary = srv, run_dir, binary
    srv.start(timeout=30.0)
    yield srv
    srv.stop()


# --------------------------------------------------------------------------
# bağlantılar
# --------------------------------------------------------------------------
class ConnectionGate(object):
    """Sunucu aynı makineden ikinci eşzamanlı bağlantıya izin vermez
    ("Çift karakterle oynamak yasak."); bu yüzden aynı anda tek istemci açılır
    ve bir kapanıştan sonra yeni bağlantı için CONNECTION_GAP saniye beklenir."""

    def __init__(self, server):
        self.server = server
        self.open = []          # açık istemciler
        self.history = []       # bu test sırasında açılan bütün istemciler
        self.quit_names = set()

    def connect(self, timeout=10.0):
        for c in list(self.open):
            self.release(c)
        wait = STATE.last_close + CONNECTION_GAP - time.monotonic()
        if wait > 0:
            time.sleep(wait)
        c = self.server.connect(timeout=timeout)
        self.open.append(c)
        self.history.append(c)
        return c

    def release(self, c):
        """Oyundaysa 'ayrıl' de, soketi kapat, kapanış zamanını kaydet."""
        if c in self.open:
            self.open.remove(c)
        try:
            if c.in_game and not c.closed and not c.closed_by_server:
                if c.quit():
                    self.quit_names.add(c.name)
            else:
                c.close()
        finally:
            STATE.last_close = time.monotonic()

    def cleanup(self):
        """Test sonu: açık istemcileri kapat, bağlantısı kopmuş (link-dead)
        karakterleri yeniden bağlanıp oyundan çıkar ki sonraki testlerin
        odası temiz kalsın."""
        for c in list(self.open):
            self.release(c)
        for c in self.history:
            if c.in_game and c.name and c.name not in self.quit_names:
                self._quit_linkdead(c.name)
        self.history = []

    def _quit_linkdead(self, name):
        try:
            c = self.connect()
            mud.login(c, name, PASSWORD)
            self.release(c)
        except mud.MudError:
            for c in list(self.open):
                self.release(c)


@pytest.fixture(scope="session")
def gate(server):
    return ConnectionGate(server)


@pytest.fixture
def client(gate):
    """Bağlantı fabrikası: ``c = client()``. Test sonunda hepsi kapatılır."""
    yield gate.connect
    gate.cleanup()


# "uc" yok: tests/e2e/fixtures/Denemeuc sabit fikstür adıyla çakışmasın.
_TR_NUMBERS = ["bir", "iki", "dort", "bes", "alti", "yedi", "sekiz", "dokuz", "on",
               "onbir", "oniki", "onuc", "ondort", "onbes", "onalti", "onyedi", "yirmi"]


@pytest.fixture(scope="session")
def new_name():
    """Oturum içinde benzersiz, kurallara uygun (2-12 ASCII harf) karakter adı üretir."""
    counter = itertools.count()

    def make():
        i = next(counter)
        if i < len(_TR_NUMBERS):
            return "Deneme" + _TR_NUMBERS[i]
        return "Deneme" + chr(ord("a") + i // 26 % 26) + chr(ord("a") + i % 26)
    return make


# --------------------------------------------------------------------------
# ekran kaydı ve anlık görüntüler
# --------------------------------------------------------------------------
class ScreenRegistry(object):
    def __init__(self, nodeid):
        self.nodeid = nodeid

    def add(self, name, text, status="screen"):
        rec = ScreenRecord(name, self.nodeid, mud.normalise_newlines(text), status=status)
        STATE.screens.append(rec)
        return rec


@pytest.fixture
def screens(request):
    """Rapora ekran eklemek için: ``screens.add("bak", scr.text)``."""
    return ScreenRegistry(request.node.nodeid)


_ASCII_MAP = str.maketrans("çğıöşüÇĞİÖŞÜ", "cgiosuCGIOSU")


def snapshot_path(name):
    return SNAPSHOT_DIR / (name.translate(_ASCII_MAP) + ".txt")


class SnapshotStore(object):
    def __init__(self, update):
        self.update = update
        SNAPSHOT_DIR.mkdir(parents=True, exist_ok=True)

    def read(self, name):
        p = snapshot_path(name)
        if not p.is_file():
            return None
        return p.read_bytes().decode("utf-8")

    def write(self, name, text):
        snapshot_path(name).write_bytes(text.encode("utf-8"))

    def check(self, name, actual, raw_text, nodeid):
        """Altın dosyayla karşılaştır; (durum, ileti) döndür ve rapora kaydet."""
        expected = self.read(name)
        rec = ScreenRecord(name, nodeid, mud.normalise_newlines(raw_text), actual=actual, expected=expected)
        STATE.screens.append(rec)
        path = snapshot_path(name)
        if self.update:
            self.write(name, actual)
            rec.status = "updated" if expected is not None else "created"
            return rec.status, "%s yazıldı" % path
        if expected is None:
            actual_path = STATE.run_dir / "actual" / path.name if STATE.run_dir else None
            if actual_path:
                actual_path.parent.mkdir(parents=True, exist_ok=True)
                actual_path.write_bytes(actual.encode("utf-8"))
            rec.status = "missing"
            return rec.status, ("Altın dosya yok: %s\n--update-snapshots ile oluşturun. "
                                "Gerçekleşen çıktı: %s" % (path, actual_path))
        if expected == actual:
            rec.status = "passed"
            return rec.status, ""
        diff = "\n".join(difflib.unified_diff(
            mud.strip_ansi(expected).split("\n"), mud.strip_ansi(actual).split("\n"),
            fromfile="beklenen: %s" % path.name, tofile="gerçekleşen: %s" % name, lineterm=""))
        raw_diff = "\n".join(difflib.unified_diff(
            expected.split("\n"), actual.split("\n"),
            fromfile="beklenen (ANSI dahil)", tofile="gerçekleşen (ANSI dahil)", lineterm=""))
        rec.status, rec.diff = "failed", diff
        message = ("Anlık görüntü '%s' altın dosyadan farklı (%s).\n"
                   "Yenilemek için: pytest tests/e2e --update-snapshots\n\n%s\n"
                   % (name, path, diff))
        if diff.count("\n") <= 3:
            # yalnızca renk kodları değişmiş olabilir; ANSI dahil farkı da göster
            message += "\n(ANSI dahil fark)\n%s\n" % raw_diff.replace("\x1b", "\\x1b")
        return rec.status, message


@pytest.fixture(scope="session")
def snapshot_store(request):
    return SnapshotStore(update_snapshots_enabled(request.config))


# --------------------------------------------------------------------------
# rapor ve hata bağlamı
# --------------------------------------------------------------------------
@pytest.hookimpl(hookwrapper=True)
def pytest_runtest_makereport(item, call):
    outcome = yield
    rep = outcome.get_result()
    if rep.when == "call" and rep.failed and STATE.server is not None:
        rep.sections.append(("sunucu günlüğü (son 2 KB)", STATE.server.log_tail()))


def pytest_sessionfinish(session, exitstatus):
    if STATE.server is None and not STATE.screens:
        return
    RUNS_DIR.mkdir(parents=True, exist_ok=True)
    meta = {
        "tarih": datetime.datetime.now().isoformat(timespec="seconds"),
        "ikili": STATE.binary or "-",
        "çalışma dizini": STATE.run_dir or "-",
        "pytest çıkış durumu": getattr(exitstatus, "name", exitstatus),
        "platform": sys.platform,
    }
    if STATE.server is not None:
        meta["_log"] = STATE.server.log_text()[-20000:]
    page = ansi2html.render_report(STATE.screens, meta)
    (RUNS_DIR / "report.html").write_text(page, encoding="utf-8")
    if STATE.run_dir is not None and STATE.run_dir.exists():
        (STATE.run_dir / "report.html").write_text(page, encoding="utf-8")
