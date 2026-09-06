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

## Testler

```bash
python3 -m pip install pytest
ctest --test-dir build --output-on-failure
```

- `unit`: `tests/unit/test_unit.c` — UTF-8 yardımcıları ve parola özeti.
- `e2e`: `tests/e2e/` — sunucuyu geçici bir dizinde başlatır, TCP üzerinden bağlanır,
  karakter yaratır, komut çalıştırır ve ekran çıktılarını `tests/e2e/snapshots/`
  altındaki altın (golden) dosyalarla karşılaştırır (görsel regresyon).

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
