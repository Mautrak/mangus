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
(fikstürler, altın ekranlar) · `script/` (autorun.sh, `discord_tools.py`: `DISCORD_TOKEN` ve
`MANGUS_ROOT` ortam değişkenleriyle) · `doc/` belgeler
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

- `printf_to_char`/`bugf` printf biçim denetimi taşır (`PRINTF_FMT`): veriyi asla biçim olarak
  geçme (`printf_to_char(ch, "%s", metin)`); `bug()` yalnızca tek tamsayı dönüşümü (`%d`/`%ld`) kabul eder. Ekran/alan
  metnini şablon olarak doldurmak için `descr_subst`/`str_fill_name`/`prog_subst` kullan.
- Ortak yardımcılar: `handler.c` `align_index`, `flag_bits_name`, `LIST_UNLINK`; `db.h`
  `find_*_index`/`read_*_vnum`; `tables.h` `yp_range`, `material_index`; `lookup.h`
  `name_table_lookup`; `utf8.h` `utf8_fit`; `prog_util.c` mob/obj prog ortak çekirdeği
  (`prog_subst`, `prog_name_in_text`, `prog_obj_owned_by`). Yenisini yazmadan önce bunlara bak.
- Açılışta `skill_table_verify` yinelenen SLOT/ad bulursa günlüğe `bug` yazar; tablo boyları
  `_Static_assert` ile kilitlidir (`MAX_SKILL`, `MAX_LANGUAGE`, `MAX_PC_RACE`).
- Bot eşikleri (`BOT_HP_*`, `BOT_AVOID_*`, `BOT_MIN/HOUR`) ve yol bulma çekirdeği (`bot_bfs`)
  `bot.h`/`bot_brain.c`'de tek yerdedir; `act_hera.c` `find_path` de `bot_bfs` kullanır.
- e2e yardımcıları: `tests/e2e/mud.py` (`m1_hash`, `write_player_file`, `wait_closed`),
  `conftest.spawn_server`; bot testleri sessiz oda 1203 ve `expect` tabanlı bekleme kullanır.

## Tuzaklar (gotcha)

**Derleyici / platform**
- gcc'nin clang'da olmayan denetimleri CI'ı kırar: glibc `warn_unused_result`
  (`system`, `link`, `fread`, `write`), `-Wmisleading-indentation` sezgiseli,
  `-Wformat-zero-length`, `-Wcalloc-transposed-args`, `-Wsign-compare` (`UMIN/UMAX` içinde
  işaretli/işaretsiz karışımı); Linux'ta `libm` ayrıca bağlanır. Bu Mac'te Homebrew
  `gcc-16` var: push'tan önce `/opt/homebrew/bin/gcc-16` ile CI bayraklarını
  (`-Wall -Wextra -Wno-unused-parameter -Wno-format-truncation -Wno-stringop-truncation
  -Wno-maybe-uninitialized -Werror -DHAVE_CRYPT=1`) kullanarak dosya dosya derle.
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

## Botlar (src/bot.c, bot_brain.c, bot_chat.c, bot_war.c, bot_god.c)

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
- Adalet kuralı: bot yalnızca oyuncunun bilebileceğini bilir. Av: oda + `tara` menzili
  (`bot_scan_prey`), hatırlanan av odaları, keşif; bölge seçimi kamusal seviye aralığı +
  kendi kesim/ölüm hafızası. Görev yaratığı ve grup arkadaşı yalnızca aynı bölgede
  (`nerede`), PK hedefi `kim` listesinden, konumu devriye/`nerede` ile. `char_list`
  üzerinden konum okuyan yeni mantık ekleme.
- Veteran kuralları (ölüm analizi: 121 ölümün yarısı seviyesinin 1-5 üstündeki avdan): yalnızken
  av en fazla seviye+1 (`level_bonus`), denk seviye tercih edilir (`bot_prey_score`), kutsanmış/
  hızlı yaratığa ve yardımcısı olan kalabalığa (`ASSIST_*`, aynı tür, saldırgan) girilmez,
  dövüşe %70 yp altında başlanmaz, %60 altında dinlenilir (%92'ye kadar), kaçış %32/%40,
  kaybedilen dövüşten erken çıkılır; ölüm: bölge 3 saat (2+ ölümde 12), yaratık 6 saat, oda
  3 saat kaçınılır; yolda bir sonraki odada seviye+3 saldırgan görünce oda 20 dk kaçınılıp
  yeniden yol bulunur (`bot_travel_scout`, `bot_room_avoided` yol bulmaya bağlı).
- Kanallar: `söyle`/`haykır`/`duygu` rol içi havuzlar (`ic_*`), `kd`/`kdg` konu dışı
  (`ooc_*`); `bot_fill_ch(..., ic)` üslubu kanala göre seçer. `kdg` (`do_kdg`,
  `COMM_NOKDG`) herkese açık konu dışı kanaldır; botlar `bot_hear(BOT_CH_KDG)` ile duyar.
- Tanrı botu (bot_god.c): kadroda `!tanrı <bot> <seviye>`; `bot_think` ve `bot_hear` en başta
  `bot_is_god` ile ayrılır; `do_pray` `BOT_CH_PRAY` kancasıyla ulaşır. Yalnızca transfer
  (dua ile sıkışan ölümlü, tapınağa), restore (yalnız insan, saatte bir), duyuru, immtalk,
  kd/kdg/söyle, goto tapınak kullanır; ölümlü botlara yardım etmez.
- Kabal (bot_war.c): kadrodaki `!lider` botu şartlar sağlanınca kabal + `PLR_CANINDUCT`
  alır (tanrı ataması, bir kez); üyelik `kd` isteği + gerçek `induct` komutuyla. PK hedefi
  `kim`'den, bölge devriyesi; `kk` yardım çağrısı ("yardım! <isim> <bölge>'de ..."). Baskın
  `BOT_ST_RAID` (`raid_step` 0-3 saldırı, 10+ savunma; `bot_set_state` substate'i sıfırladığı
  için ayrı alan); `act_obj.c` altar kancası `bot_cabal_alarm()`. Karargâh muhafızları 90+
  seviyedir (İstila kumandanı: haste+sanctuary): baskın 60. seviye ve 3 çevrimiçi üye ister,
  yolda seviyesinin 5+ üstünde muhafız görünce geri çekilir, baskında ölüm hedef kabalı 12 saat
  kapatır. İstila karargâhı tek yönlü iniştir (562→564), çıkış 568'deki portallar; yol bulma
  yalnızca `allow_cabal` ile oraya girer (`botlar yol <bot> <oda> kabal`).
