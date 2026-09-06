# -*- coding: utf-8 -*-
"""Mangus uçtan uca testleri için yardımcılar (yalnızca standart kütüphane).

* ``MudServer``  : çalışma dizinini hazırlar, sunucuyu başlatır/durdurur.
* ``MudClient``  : telnet benzeri TCP istemcisi; ``expect``/``read_idle``.
* ``create_character`` / ``login`` : oturum açma akışları.

Sunucu ``\\n\\r`` satır sonları, UTF-8 metin, ANSI renk dizileri ve
telnet müzakere baytları (IAC ...) gönderir; istemci bunları ayıklar.
"""
import re
import shutil
import socket
import subprocess
import time
from pathlib import Path

READY_MARKER = "kullanıma hazır"
NAME_PROMPT = "Hangi isimle anılmak istersin? "
PASSWORD_PROMPT = "Parola: "
PAGER = "[Devam etmek için ENTER]"
WRONG_PASSWORD = "Yanlış şifre."
BAD_NAME = "Kurallara uygun olmayan isim"
QUIT_MESSAGE = "Her güzel şeyin bir sonu vardır."
CONFIRM_NEW_NAME = "Doğru anladım mı, "
PASSWORD = "sifre123"      # testlerin ortak parolası (>= 5 karakter)
# Oyun içi komut istemi: "Yp:20/20 Mp:100/100 Zp:100/100 <0>\x1b[0;37m "
PROMPT_RE = re.compile(r"Yp:\d+/\d+[^\r\n]*?<\d+>(?:\x1b\[[0-9;]*m)* ")

ANSI_RE = re.compile(r"\x1b\[[0-9;?]*[ -/]*[@-~]")

IAC, DONT, DO, WONT, WILL, SB, SE = 255, 254, 253, 252, 251, 250, 240


class MudError(AssertionError):
    """Sunucuyla konuşurken oluşan hata (pytest'te assert gibi raporlanır)."""


class MudTimeout(MudError):
    pass


class MudClosed(MudError):
    pass


class MudLoginError(MudError):
    pass


def find_free_port():
    """Boş bir TCP portu seç (0'a bind edip kapatarak)."""
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as s:
        s.bind(("127.0.0.1", 0))
        return s.getsockname()[1]


def _split_telnet(data):
    """Telnet komutlarını ayıkla; sonda kalan yarım diziyi geri ver.

    Döndürür: (temiz_baytlar, bekleyen_kuyruk)
    """
    out = bytearray()
    i, n = 0, len(data)
    while i < n:
        b = data[i]
        if b != IAC:
            out.append(b)
            i += 1
            continue
        if i + 1 >= n:                       # tek başına IAC: devamı gelecek
            return bytes(out), data[i:]
        cmd = data[i + 1]
        if cmd == IAC:                       # IAC IAC -> gerçek 0xFF baytı
            out.append(IAC)
            i += 2
        elif cmd in (WILL, WONT, DO, DONT):
            if i + 2 >= n:
                return bytes(out), data[i:]
            i += 3
        elif cmd == SB:                      # alt müzakere: IAC SE'ye kadar at
            j = data.find(bytes([IAC, SE]), i + 2)
            if j < 0:
                return bytes(out), data[i:]
            i = j + 2
        else:                                # GA, NOP vb.: iki bayt
            i += 2
    return bytes(out), b""


def strip_telnet(data):
    """Bayt dizisinden bütün telnet komutlarını çıkar."""
    clean, _tail = _split_telnet(bytes(data))
    return clean


def strip_ansi(text):
    """ANSI kaçış dizilerini (ESC[...m vb.) metinden çıkar."""
    return ANSI_RE.sub("", text)


def normalise_newlines(text):
    """Sunucunun ``\\n\\r`` (ve ``\\r\\n``) satır sonlarını ``\\n`` yap."""
    return text.replace("\n\r", "\n").replace("\r\n", "\n").replace("\r", "")


def _printable(text, limit=2000):
    text = text[-limit:]
    return text.encode("utf-8", "replace").decode("utf-8", "replace")


class Screen(object):
    """Sunucudan alınan bir ekran parçası (telnet baytları ayıklanmış)."""

    def __init__(self, raw, matched=None, match=None):
        self.raw = bytes(raw)
        self.matched = matched      # eşleşen desen (str / regex) veya None
        self.match = match          # re.Match veya None

    @property
    def text(self):
        return self.raw.decode("utf-8", "replace")

    @property
    def plain(self):
        return strip_ansi(self.text)

    @property
    def lines(self):
        return normalise_newlines(self.plain).split("\n")

    def __contains__(self, needle):
        return needle in self.text

    def __str__(self):
        return self.text

    def __repr__(self):
        return "Screen(%r)" % _printable(self.text, 200)

    def __add__(self, other):
        return Screen(self.raw + other.raw, other.matched, other.match)


class MudServer(object):
    """Sunucu süreci ve çalışma dizini."""

    RUNTIME_DIRS = ("player", "gods", "remort", "log", "log/ip", "log/kanal", "log/events", "data")

    def __init__(self, binary, repo_root, run_dir, port=None, host="127.0.0.1"):
        self.binary = Path(binary)
        self.repo_root = Path(repo_root)
        self.run_dir = Path(run_dir)
        self.host = host
        self.port = port or find_free_port()
        self.log_path = self.run_dir / "server.log"
        self.proc = None
        self._log_handle = None

    # ---- hazırlık --------------------------------------------------------
    def prepare(self, runtime_dirs=None):
        """Depodaki area/ dizinini ve veri dosyalarını çalışma dizinine kopyala."""
        if runtime_dirs is not None:
            self.RUNTIME_DIRS = tuple(runtime_dirs)
        self.run_dir.mkdir(parents=True, exist_ok=True)
        src_area = self.repo_root / "area"
        dst_area = self.run_dir / "area"
        if dst_area.exists():
            shutil.rmtree(dst_area)
        shutil.copytree(src_area, dst_area)
        # Depodaki log/ düzeni aynen kurulur: log/kanal eksikse 'söyle' vb. kanal
        # komutları sunucuyu çökertir (data.c write_channel_log, NULL FILE*).
        for name in self.RUNTIME_DIRS:
            (self.run_dir / name).mkdir(parents=True, exist_ok=True)
        ud = self.repo_root / "data" / "ud_data"
        if ud.exists():
            shutil.copy(ud, self.run_dir / "data" / "ud_data")
        return self

    # ---- yaşam döngüsü ---------------------------------------------------
    def start(self, timeout=30.0):
        if not self.binary.exists():
            raise MudError("Sunucu ikilisi yok: %s" % self.binary)
        cwd = self.run_dir / "area"
        self._log_handle = open(str(self.log_path), "wb")
        self.proc = subprocess.Popen(
            [str(self.binary), str(self.port)],
            cwd=str(cwd),
            stdin=subprocess.DEVNULL,
            stdout=self._log_handle,
            stderr=subprocess.STDOUT,
        )
        self.wait_ready(timeout)
        return self

    def wait_ready(self, timeout=30.0):
        deadline = time.monotonic() + timeout
        while True:
            text = self.log_text()
            if READY_MARKER in text:
                return
            code = self.proc.poll()
            if code is not None:
                raise MudError(
                    "Sunucu hazır olmadan sonlandı (çıkış kodu %s).\n--- günlük ---\n%s"
                    % (code, self.log_tail())
                )
            if time.monotonic() > deadline:
                raise MudTimeout(
                    "Sunucu %.0f saniyede hazır olmadı (%r bekleniyordu).\n--- günlük ---\n%s"
                    % (timeout, READY_MARKER, self.log_tail())
                )
            time.sleep(0.2)

    def stop(self):
        proc, self.proc = self.proc, None
        if proc is not None and proc.poll() is None:
            try:
                proc.terminate()
                proc.wait(timeout=10)
            except Exception:
                try:
                    proc.kill()
                    proc.wait(timeout=10)
                except Exception:
                    pass
        if self._log_handle is not None:
            try:
                self._log_handle.close()
            except Exception:
                pass
            self._log_handle = None

    def is_running(self):
        return self.proc is not None and self.proc.poll() is None

    # ---- günlük ----------------------------------------------------------
    def log_bytes(self):
        try:
            with open(str(self.log_path), "rb") as fh:
                return fh.read()
        except OSError:
            return b""

    def log_text(self):
        return self.log_bytes().decode("utf-8", "replace")

    def log_tail(self, nbytes=2048):
        return self.log_bytes()[-nbytes:].decode("utf-8", "replace")

    # ---- yollar ----------------------------------------------------------
    @property
    def player_dir(self):
        return self.run_dir / "player"

    def player_file(self, name):
        return self.player_dir / (name[:1].upper() + name[1:])

    def connect(self, timeout=10.0):
        return MudClient(self.host, self.port, timeout=timeout, server=self)


class MudClient(object):
    """Sunucuya tek bir TCP bağlantısı."""

    def __init__(self, host, port, timeout=10.0, server=None):
        self.host, self.port = host, port
        self.server = server
        self.sock = socket.create_connection((host, port), timeout=timeout)
        self._pending = b""          # telnet açısından yarım kalmış kuyruk
        self._buf = bytearray()      # ayıklanmış, henüz tüketilmemiş baytlar
        self.transcript = bytearray()  # alınan her şey (ayıklanmış)
        self.closed_by_server = False
        self.closed = False
        self.greeted = False
        self.in_game = False
        self.name = None
        self.quit_ok = False

    # ---- düşük seviye ----------------------------------------------------
    def _recv_once(self, timeout):
        """En fazla ``timeout`` saniye bekleyerek bir parça oku."""
        if self.closed_by_server:
            return False
        self.sock.settimeout(max(0.01, timeout))
        try:
            chunk = self.sock.recv(65536)
        except socket.timeout:
            return False
        except (ConnectionResetError, ConnectionAbortedError, OSError):
            self.closed_by_server = True
            return False
        if not chunk:
            self.closed_by_server = True
            return False
        clean, self._pending = _split_telnet(self._pending + chunk)
        self._buf += clean
        self.transcript += clean
        return True

    def _decoded(self):
        return bytes(self._buf).decode("utf-8", "surrogateescape")

    def _search(self, patterns):
        """Desenler arasından tamponda EN ERKEN eşleşeni bul."""
        text = self._decoded()
        best = None
        for pat in patterns:
            if isinstance(pat, str):
                idx = text.find(pat)
                if idx >= 0:
                    cand = (idx, idx + len(pat), pat, None)
                else:
                    continue
            else:
                m = pat.search(text)
                if not m:
                    continue
                cand = (m.start(), m.end(), pat, m)
            if best is None or cand[0] < best[0]:
                best = cand
        if best is None:
            return None
        _start, end, pat, m = best
        nbytes = len(text[:end].encode("utf-8", "surrogateescape"))
        raw = bytes(self._buf[:nbytes])
        del self._buf[:nbytes]
        return Screen(raw, matched=pat, match=m)

    def _context(self):
        ctx = "--- istemci tamponu (son 2 KB) ---\n%s\n" % _printable(self._decoded())
        if self.server is not None:
            ctx += "--- sunucu günlüğü (son 2 KB) ---\n%s\n" % self.server.log_tail()
        return ctx

    # ---- yüksek seviye ---------------------------------------------------
    def send(self, line):
        """Bir satır gönder (UTF-8 + CRLF)."""
        self.send_raw(line.encode("utf-8") + b"\r\n")

    def send_raw(self, data):
        try:
            self.sock.sendall(data)
        except OSError as exc:
            raise MudClosed("Gönderim başarısız (%s).\n%s" % (exc, self._context()))

    def expect(self, pattern, timeout=5.0):
        """``pattern`` (str, regex veya bunların listesi) gelene dek oku.

        Eşleşmenin sonuna kadar olan metni ``Screen`` olarak döndürür ve
        tampondan tüketir. Zaman aşımında/kapanmada ``MudError`` fırlatır.
        """
        patterns = list(pattern) if isinstance(pattern, (list, tuple)) else [pattern]
        deadline = time.monotonic() + timeout
        while True:
            scr = self._search(patterns)
            if scr is not None:
                return scr
            if self.closed_by_server:
                raise MudClosed(
                    "Sunucu bağlantıyı kapattı; %s bekleniyordu.\n%s"
                    % (_describe(patterns), self._context())
                )
            remaining = deadline - time.monotonic()
            if remaining <= 0:
                raise MudTimeout(
                    "%.1f saniyede %s gelmedi.\n%s"
                    % (timeout, _describe(patterns), self._context())
                )
            self._recv_once(min(remaining, 0.25))

    def read_idle(self, idle=0.4, max_wait=5.0):
        """``idle`` saniye boyunca veri gelmeyene dek (en çok ``max_wait``) oku."""
        deadline = time.monotonic() + max_wait
        while time.monotonic() < deadline:
            if not self._recv_once(min(idle, max(0.01, deadline - time.monotonic()))):
                break                       # idle süresi doldu ya da bağlantı kapandı
        raw = bytes(self._buf)
        del self._buf[:]
        return Screen(raw)

    def wait_greeting(self, timeout=10.0):
        scr = self.expect(NAME_PROMPT, timeout=timeout)
        self.greeted = True
        return scr

    def command(self, cmd, timeout=10.0):
        """Oyun içi komut gönder; sayfalayıcıyı geçerek istemi bekle."""
        self.send(cmd)
        return self.read_until_prompt(timeout=timeout)

    def read_until_prompt(self, timeout=10.0, max_pages=50):
        """Komut istemine kadar oku; ``[Devam etmek için ENTER]`` gelirse ilerle."""
        combined = Screen(b"")
        for _ in range(max_pages):
            scr = self.expect([PROMPT_RE, PAGER], timeout=timeout)
            combined = combined + scr
            if scr.matched is PAGER:
                self.send("")
                continue
            return combined
        raise MudError("Sayfalayıcı %d sayfadan sonra da bitmedi.\n%s" % (max_pages, self._context()))

    def quit(self, timeout=5.0):
        """Oyundaysa 'ayrıl' ile çık; her durumda soketi kapat.

        Sunucunun veda iletisi görüldüyse True döndürür."""
        try:
            if self.in_game and not self.closed and not self.closed_by_server:
                self.send("ayrıl")
                try:
                    self.expect(QUIT_MESSAGE, timeout=timeout)
                    self.quit_ok = True
                except MudError:
                    pass
                self.in_game = False
                # sunucunun bağlantıyı kapatmasını kısa süre bekle
                deadline = time.monotonic() + 2.0
                while not self.closed_by_server and time.monotonic() < deadline:
                    self._recv_once(0.2)
        finally:
            self.close()
        return self.quit_ok

    def close(self):
        """Soketi kapat (oyundaysa karakter sunucuda bağlantısız kalır)."""
        self.closed = True
        try:
            self.sock.close()
        except OSError:
            pass


def _describe(patterns):
    parts = []
    for p in patterns:
        parts.append(repr(p) if isinstance(p, str) else "regex %r" % p.pattern)
    return " / ".join(parts)


def create_character(client, name, password, race="insan", klass="savaşçı",
                     sex="E", alignment="i", ethos="t", timeout=10.0):
    """Yeni karakter oluşturup oyuna sok; tüm ekranların metnini döndür."""
    parts = []
    if not client.greeted:
        parts.append(client.wait_greeting(timeout))
    client.send(name)
    parts.append(client.expect("Doğru anladım mı, %s (E/H)? " % name, timeout))
    client.send("E")
    parts.append(client.expect("karakteri için bir şifre girin: ", timeout))
    client.send(password)
    parts.append(client.expect("Lütfen şifreyi tekrar girin: ", timeout))
    client.send(password)
    parts.append(client.expect("Irkınız ne olsun?", timeout))
    client.send(race)
    parts.append(client.expect("cinsiyeti ne olsun ( E - K )? ", timeout))
    client.send(sex)
    parts.append(client.expect("Sınıfın ne olsun", timeout))
    client.send(klass)
    parts.append(client.expect("( i - y - k )? ", timeout))
    client.send(alignment)
    parts.append(client.expect(PAGER, timeout))
    client.send("")
    parts.append(client.expect("( t - y - k )? ", timeout))
    client.send(ethos)
    parts.append(client.expect(PAGER, timeout))
    client.send("")
    parts.append(client.expect(PAGER, timeout))          # "genel" yardımı
    client.send("")
    parts.append(client.expect(PAGER, timeout))          # MOTD
    client.send("")
    parts.append(client.read_until_prompt(timeout))      # hoşgeldin + oda
    client.in_game = True
    client.name = name
    return "".join(p.text for p in parts)


def login(client, name, password, timeout=10.0):
    """Var olan (dosyadan ya da bağlantısı kopmuş) karakterle oyuna gir."""
    parts = []
    if not client.greeted:
        parts.append(client.wait_greeting(timeout))
    client.send(name)
    scr = client.expect([PASSWORD_PROMPT, CONFIRM_NEW_NAME], timeout)
    parts.append(scr)
    if scr.matched == CONFIRM_NEW_NAME:
        raise MudLoginError("%r adında kayıtlı/oyunda karakter yok (yeni karakter akışı başladı).\n%s"
                            % (name, client._context()))
    client.send(password)
    for _ in range(6):
        scr = client.expect([PROMPT_RE, PAGER, WRONG_PASSWORD], timeout)
        parts.append(scr)
        if scr.matched is PAGER:
            client.send("")
            continue
        if scr.matched == WRONG_PASSWORD:
            raise MudLoginError("Giriş başarısız: %r için yanlış şifre.\n%s"
                                % (name, client._context()))
        break
    client.in_game = True
    client.name = name
    return "".join(p.text for p in parts)
