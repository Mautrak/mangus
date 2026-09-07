# Mangus — geliştirici / Claude Code notları

Uzak Diyarlar (ROM 2.4 / Anatolia 2.1) tabanlı Türkçe MUD sunucusu. Kaynak C11'dir
(`-std=gnu11`), tüm dosyalar ve ağ trafiği UTF-8'dir. Linux (gcc), macOS (clang) ve
Windows (MSYS2 UCRT64 MinGW-w64 gcc) üzerinde `-Wall -Wextra -Werror` ile derlenir.

## Komutlar

```bash
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=RelWithDebInfo -DMANGUS_WERROR=ON
cmake --build build --parallel            # -> build/mangus (Windows: mangus.exe)
make -C src                               # CMake'siz hızlı POSIX derlemesi
./build/test_unit                         # birim testleri
MANGUS_BIN=$PWD/build/mangus .venv/bin/python -m pytest -q tests/e2e            # e2e + snapshot (~50 s)
MANGUS_BIN=$PWD/build/mangus .venv/bin/python -m pytest -q tests/e2e --update-snapshots
cd area && ../build/mangus 4000           # sunucu; çalışma dizini area/ OLMALI
```

`.venv` yoksa: `python3 -m venv .venv && .venv/bin/pip install pytest`.
Bu Mac'te `cmake`/`ninja` PATH'te yoktur (`pip install cmake ninja` ile bir venv'e kurulur);
Homebrew ve `gh` `/opt/homebrew/bin` altındadır.

## Dizinler

`src/` sunucu · `area/` alan dosyaları ve yardım metinleri · `tests/unit`, `tests/e2e`
(fikstürler, altın ekranlar) · `script/` (autorun.sh, Discord araçları) · `doc/` belgeler
ve lisanslar · `.github/workflows/ci.yml` CI. Çalışma zamanı dizinleri (`player/`, `gods/`,
`remort/`, `log/`, `data/`) `area/`'ya göre `../` ile bulunur ve açılışta oluşturulur.

## Kurallar

- İşletim sistemine bağımlı kod yalnızca `src/platform.c`'de yaşar (soket, zaman, uyku,
  dizin, dosya taşıma). Yeni POSIX çağrısı eklemeden önce MinGW'de var mı bak.
- Mekanik/toplu değişiklikler (yeniden kodlama, biçimlendirme, betikle dönüşüm) ayrı
  commit olur; commit mesajları Türkçe'dir.
- `data/ud_data` ve `area/area_stat.txt` git'te izlenir ama sunucu çalışınca yeniden
  yazılır: yerel koşudan sonra `git checkout -- data/ud_data area/area_stat.txt`.
- Ekran çıktısını değiştiren her değişiklikten sonra snapshot altın dosyalarını
  `--update-snapshots` ile yenile ve farkı gözden geçir.

## Tuzaklar (gotcha)

**Derleyici / platform**
- gcc'nin clang'da olmayan denetimleri CI'ı kırar: glibc `warn_unused_result`
  (`system`, `link`, `fread`, `write`), `-Wmisleading-indentation` sezgiseli,
  `-Wformat-zero-length`, `-Wcalloc-transposed-args`; Linux'ta `libm` ayrıca bağlanır.
  Yerelde clang ile `-Wimplicit-fallthrough -Wformat-zero-length` ekleyerek yaklaş.
- Win64'te `long` 32 bittir: işaretçiyi asla `long`'a çevirme (allocator `intptr_t`
  kullanır); `time_t` 64 bittir (`%lld` + dönüşüm); `rename()` hedef varsa başarısız
  olur → `platform_replace_file()`; `link()`, `crypt()`, `fork()`, `SIGPIPE` yok;
  `select(0,…)` uyku olarak kullanılamaz (`platform_sleep_us`); `FD_SETSIZE`
  `winsock2.h`'den önce tanımlanır; `rand_s` için `_CRT_RAND_S`.
- Windows CI'da CMake runner'ın Python'unu bulur (pytest yok); iş akışı
  `Python3_EXECUTABLE`'ı MSYS2 Python'una sabitler.
- Ninja ilk hatalarda durur: kırmızı bir koşu sonraki hataları gizleyebilir;
  `gh run view <id> --log-failed` ile bak.

**UTF-8 / Türkçe**
- Alanlar UTF-8 olunca dizgi alanı taştı; `MAX_STRING` (db.c) 8 MB.
- Latin-5'e göre boyutlanmış küçük tamponlar taşar (`who_name[8]`, `oyuncukatli[16]`).
- `capitalize()` ASCII kalmalıdır ('i'→'I'): oyuncu dosya adları ondan türer
  ("Ismail"). Cümle başı için `utf8_upper_first()`.
- `str_cmp/str_prefix` I/ı/İ/i'yi eşdeğer sayar; tablo aramalarında ilk harf
  `utf8_first_eq` ile karşılaştırılır (bayt karşılaştırması çok baytlı harfte bozulur);
  sütun hizası `%-*s` + `utf8_width()`.
- `fread_string` eski Latin-5 oyuncu/alan dosyalarını okurken UTF-8'e çevirir;
  yazma tarafı yalnızca UTF-8. Girdi filtresi Latin-5 girdiyi de çevirir.
- Python `re.sub` yer değiştirme dizgisinde `\n`, `\0` yorumlanır; sabit metin için
  lambda kullan (comm.c/save.c iki kez bozuldu).

**Sunucu davranışı**
- Seviye 2'nin altındaki karakterler hiç kaydedilmez (`save_char_obj`); testler
  `tests/e2e/fixtures/Denemeuc` (seviye 2, parola `sifre123`) ve `Denemeuc.legacy`
  (DES özeti + Latin-5 ünvan) fikstürlerini kullanır.
- Aynı hosttan ikinci eşzamanlı bağlantı, biri oyundayken reddedilir ("Çift karakterle
  oynamak yasak"): testler tek tek ve 0,5 s arayla bağlanır.
- Sayfalanmış çıktı (`komutlar`) bir sonraki girdi satırını sayfalayıcı yanıtı sayar.
- `../log/kanal/` yokken kanal komutları NULL `FILE*` ile çökerdi (düzeltildi);
  `data.c`'de `fopen` sonucunu her zaman denetle.
- Komutlar: çıkış `ayrıl`, skor `skor`, yardım `yardım`, kim `kim`; `çık` eşya çıkarır.
- Parolalar `$m1$` (tuzlu SHA-256); eski `crypt(3)` özetleri POSIX'te girişte yükseltilir,
  Windows'ta doğrulanamaz (CI'da o test atlanır: 24 geçti, 1 atlandı normaldir).

**Bilinen, dokunulmamış eski hatalar** (oynanış kararı): `do_lore` içinde erişilemeyen
etki listesi (act_obj.c), `spell_dispel_magic` her zaman başarılı der (`found = TRUE`),
`spell_entangle`'da kullanılmayan `dam`, `QuestPractice` koşulsuz kaydedilir (save.c),
12 karakterden uzun komut adları `komutlar` sütununa sığmaz.

## Botlar (src/bot.c, bot_brain.c, bot_chat.c)

- Botlar descriptor'sız PC'lerdir: `IS_BOT(ch)` (`pcdata->bot != NULL`). `who`/`whois`/
  `where` listeleri `bot_who_collect()` ile hem soketleri hem botları gezer; `kd`, `söyle`,
  `haykır`, `ganlat`, `kk` ve sosyaller `bot_hear()` kancasıyla botlara ulaşır.
- `bot_update()` her pulse'ta `wait`/`daze` azaltır, `timer`'ı sıfırlar (hiçliğe
  sürüklenme yok) ve ~saniyede bir `bot_think()` çağırır. Tek karar = tek komut.
- Botlar `act()` çıktısı almaz; kararlar veri yapılarından verilir. Oyun çıktısına
  dayanan mantık yazma.
- Kadro `area/botlar.txt`; testler onu kopyadan siler (`MudServer.prepare(bots=False)`).
  Bot testleri kendi sunucusunu `bots=True` ile açar (`tests/e2e/test_bots.py`).
- Bot adı `check_parse_name()` ile denetlenir (yaratık anahtar kelimesiyle çakışan ad
  devre dışı kalır, günlüğe yazılır). Oyuncu dosyasında `Bot 1` satırı olmayan aynı
  adlı dosya varsa bot devre dışı kalır (gerçek oyuncu korunur).
- Hata ayıklama: ölümsüz olarak `botlar debug` (komut ve durum geçişleri günlüğe),
  `botlar av <isim>` (av bölgesi adayları), `botlar <isim>` (durum, yol, istatistik).
