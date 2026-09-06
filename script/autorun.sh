#!/bin/sh
# Mangus sunucusunu yeniden başlatma döngüsüyle çalıştırır.
# Kullanım: script/autorun.sh [port]   (varsayılan port 4000)
# Sunucu area/ dizininden çalışır; ikili build/mangus ya da src/mangus'tan bulunur.
set -u
ROOT=$(cd "$(dirname "$0")/.." && pwd)
PORT=${1:-4000}
BIN="$ROOT/build/mangus"
[ -x "$BIN" ] || BIN="$ROOT/src/mangus"
[ -x "$BIN" ] || { echo "mangus ikilisi bulunamadı; önce derleyin (cmake -B build && cmake --build build)." >&2; exit 1; }

mkdir -p "$ROOT/log"
cd "$ROOT/area" || exit 1
rm -f shutdown.txt

while :; do
    index=1000
    while [ -e "$ROOT/log/$index.log" ]; do index=$((index + 1)); done
    "$BIN" "$PORT" > "$ROOT/log/$index.log" 2>&1
    if [ -e shutdown.txt ]; then
        rm -f shutdown.txt
        exit 0
    fi
    sleep 10
done
