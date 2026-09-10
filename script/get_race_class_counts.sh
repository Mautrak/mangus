#!/bin/sh
# Oyuncu dosyalarından ırk ve sınıf dağılımını sayar (çoktan aza).
# Kullanım: script/get_race_class_counts.sh [oyuncu_dizini]   (varsayılan: <depo>/player)
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
PLAYER_DIR=${1:-$ROOT/player}

[ -d "$PLAYER_DIR" ] || { echo "Oyuncu dizini yok: $PLAYER_DIR" >&2; exit 1; }

echo "Irklar:"
grep -h '^Race ' "$PLAYER_DIR"/* 2>/dev/null | awk '{ sub(/~$/, "", $2); print $2 }' \
    | sort | uniq -c | sort -rn

echo "Sınıflar:"
# Sınıf numarası -> ad eşlemesi src/const.c class_table sırasıdır (CLASS_* sabitleri, merc.h).
grep -h '^Cla ' "$PLAYER_DIR"/* 2>/dev/null | awk '
    BEGIN { split("gan ermiş hırsız savaşçı adbolar kembolar ninja korucu biçimci samuray vampir karakam ögeci", ad, " ") }
    { n = $2 + 1; print (n in ad) ? ad[n] : "sınıf#" $2 }' \
    | sort | uniq -c | sort -rn
