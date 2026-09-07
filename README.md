# Mangus

Mangus, [Uzak Diyarlar](https://github.com/yelbuke/UzakDiyarlar) 2.1b2 (ROM 2.4 / Anatolia 2.1)
tabanlı, Türkçe bir metin tabanlı çok oyunculu rol yapma oyunudur (MUD).

Sunucu C11 ile yazılmıştır; Linux, macOS (Apple Silicon dahil) ve Windows'ta
yerel (native) olarak derlenir. Tüm metinler, alan dosyaları ve ağ trafiği **UTF-8**'dir.

## Dizinler

| Dizin       | İçerik                                                        |
|-------------|---------------------------------------------------------------|
| `src/`      | Sunucu kaynak kodu (C11)                                      |
| `area/`     | Alan dosyaları (`*.are`), `area.lst`, yardım metinleri        |
| `tests/`    | Birim testleri (C) ve uçtan uca / görsel regresyon testleri (Python) |
| `script/`   | Yardımcı betikler (`autorun.sh`, Discord araçları, yedekleme) |
| `doc/`      | Belgeler, oyuncu el kitabı, Anatolia/ROM lisans ve notları    |
| `.github/`  | GitHub Actions CI tanımı                                      |

Çalışma zamanında `player/`, `gods/`, `remort/`, `log/` ve `data/` dizinleri
sunucu tarafından gerektiğinde oluşturulur (git'e alınmazlar).

## Derleme

Gereksinimler: bir C11 derleyicisi (clang, gcc veya MinGW-w64 gcc), CMake ≥ 3.16.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

İkili `build/mangus` (Windows'ta `build/mangus.exe`) olarak üretilir.
CMake olmadan hızlı bir POSIX derlemesi için:

```bash
make -C src
```

Windows'ta [MSYS2](https://www.msys2.org) UCRT64 ortamında
`mingw-w64-ucrt-x86_64-gcc`, `-cmake` ve `-ninja` paketleriyle aynı CMake komutları çalışır.

## Çalıştırma

Sunucu, çalışma dizini olarak `area/` dizinini bekler; diğer dizinlere göreli yollarla erişir.

```bash
cd area && ../build/mangus 4000
```

Yeniden başlatma döngüsüyle çalıştırmak için:

```bash
script/autorun.sh 4000
```

Günlükler `log/<numara>.log` dosyalarına yazılır. Kapatmak için oyun içindeki
`shutdown` komutu `area/shutdown.txt` oluşturur ve döngü sona erer.

İstemcinizin karakter setini **UTF-8** olarak ayarlayın. Hâlâ ISO-8859-9 gönderen
eski istemcilerin girdisi sunucuda otomatik olarak UTF-8'e çevrilir.

## Botlar

`area/botlar.txt` varsa sunucu açılışta bir bot kadrosu yükler: botlar gerçek oyuncu
karakterleridir (`player/` altında kaydedilir, seviye atlar, pratik ve eğitim yapar,
alışveriş ve görev yapar, cesetlerini toplar, kabal savaşına girer) ve `kim`, `kimdir`,
`nerede` listelerinde görünür; `söyle`, `haykır`, `kd`, `kdg` ve sosyallere Türkçe cevap
verir, insan oyuncuyu gruba davet eder ya da "gel/grup" dendiğinde takip eder. Soketleri
yoktur; her oyun pulse'ında `bot_update()` (src/bot.c) çalışır ve komutlarını sıradan
oyuncu komutu olarak `interpret()` üzerinden verir.

Botlar hile yapmaz: dünya listesine bakmaz; odasını, `tara` menzilini, aynı bölgedeki
oyuncuları (`nerede`), `kim` listesini ve kendi hafızasını (av odaları, bölge deneyimi,
görülen düşmanlar) kullanır; girişte ışınlanmaz, sıkışınca `anımsa` eder ya da yardım
ister. `söyle`/`haykır`/`duygu` yalnızca rol içi (diyarın dili), `kd`/`kdg` konu dışı;
`ganlat`/`kk` taktik konuşmadır. `kdg` herkese açık konu dışı kanaldır (`kdg` yazarak
açılıp kapanır).

Kabal yaşamı: kadrodaki `!lider <bot> <kabal>` satırı, tanrıların o botu (20. seviyeye
ulaşıp görevciden katil hakkı alınca) kabal lideri atadığı anlamına gelir. Diğer botlar ve
oyuncular lidere `kd <lider> <kabal> kabalına katılmak istiyorum` diyerek başvurur; lider
oyunun kurallarını (katil hakkı, seviye, sınıf/yönelim uyumu) uygulayıp gerçek `induct`
komutuyla üye alır. Kabal üyeleri `kim` listesinden düşman kabal üyelerini seçip
bölgelerde devriye gezer, saldırıya uğrayınca `kk` ile yardım ister (arkadaşları gelir),
zaman zaman karargâh baskını düzenler (düşman kabal eşyasını sunağından alıp kendi
karargâhına bırakır) ve çalınan kendi eşyalarının peşine düşer.

Kadro dosyasının biçimi dosyanın başında açıklanmıştır (`isim|ırk|sınıf|cinsiyet|yönelim|
etik|kişilik|saat|saat|Türkçe|küçük harf|katil|lakap`); `!enaz`/`!encok` aynı anda
çevrimiçi bot sayısını sınırlar, `!lider` kabal liderlerini belirler. Dosyayı silmek botları tamamen kapatır. Ölümsüzler
`botlar`, `botlar <isim>`, `botlar av <isim>`, `botlar bağla/ayır <isim>` ve
`botlar debug` komutlarını kullanabilir. Bot adları oyuncular tarafından alınamaz;
bot oyuncu dosyalarında `Bot 1` satırı bulunur.

## Testler

```bash
python3 -m pip install pytest
ctest --test-dir build --output-on-failure
```

- `unit`: `tests/unit/test_unit.c` — UTF-8 yardımcıları ve parola özeti.
- `e2e`: `tests/e2e/` — sunucuyu geçici bir dizinde başlatır, TCP üzerinden bağlanır,
  karakter yaratır, komut çalıştırır ve ekran çıktılarını `tests/e2e/snapshots/`
  altındaki altın (golden) dosyalarla karşılaştırır (görsel regresyon). Botlu sunucu
  testleri `tests/e2e/test_bots.py` içindedir; diğer testler bot kadrosunu kapalı tutar.

Yalnızca uçtan uca testleri çalıştırmak ya da altın dosyaları yenilemek için:

```bash
MANGUS_BIN=build/mangus python3 -m pytest -q tests/e2e
MANGUS_BIN=build/mangus python3 -m pytest -q tests/e2e --update-snapshots
```

Her çalıştırma `tests/e2e/_runs/report.html` dosyasına renkli bir ekran raporu yazar.

## Sürekli tümleştirme

`.github/workflows/ci.yml`, her push ve pull request için Ubuntu (gcc), macOS
(Apple Silicon, clang) ve Windows (MinGW-w64 gcc) üzerinde `-Werror` ile derler,
birim ve uçtan uca testleri çalıştırır; üretilen ikilileri ve görsel raporu
artefakt olarak yükler.

## Parolalar

Parolalar tuzlu, yinelemeli SHA-256 ile (`$m1$...`) saklanır; bu biçim her
platformda aynıdır. Eski `crypt(3)` özetleri Linux ve macOS'ta girişte doğrulanır
ve başarılı girişte yeni biçime yükseltilir. Windows'ta eski özetler doğrulanamaz.

## Lisans

Diku, Merc, ROM ve Anatolia lisansları `doc/Anatolia belgeleri/` altındadır ve
geçerliliğini korur.
