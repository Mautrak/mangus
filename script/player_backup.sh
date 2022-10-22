#!/bin/bash
DATE=$(date +%Y-%m-%d-%H%M%S)
BACKUP_DIR="$HOME/backup"
SOURCE="$HOME/mud/mangus/player"
tar -cvzpf $BACKUP_DIR/player-backup-$DATE.tar.gz $SOURCE