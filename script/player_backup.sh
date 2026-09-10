#!/bin/sh
# Oyuncu dosyalarını tarihli bir tar.gz yedeğine alır.
# Kullanım: script/player_backup.sh [yedek_dizini]
#   kaynak: MANGUS_PLAYER_DIR ya da <depo>/player; yedek dizini varsayılan ~/backup
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
SOURCE=${MANGUS_PLAYER_DIR:-$ROOT/player}
BACKUP_DIR=${1:-$HOME/backup}
DATE=$(date +%Y-%m-%d-%H%M%S)
ARCHIVE="$BACKUP_DIR/player-backup-$DATE.tar.gz"

[ -d "$SOURCE" ] || { echo "Oyuncu dizini yok: $SOURCE" >&2; exit 1; }
mkdir -p "$BACKUP_DIR"
tar -czpf "$ARCHIVE" -C "$(dirname "$SOURCE")" "$(basename "$SOURCE")"
echo "$ARCHIVE"
