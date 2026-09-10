#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""Mangus Discord araçları (discord.py 2.x).

Kullanım:
  script/discord_tools.py check-user [<kullanıcı-id>]   kullanıcı adını yazdırır
  script/discord_tools.py members                        sunucu üyelerinin kimliklerini
                                                         data/discord_users dosyasına yazar
  script/discord_tools.py feed-events [--window SN]      log/events/events içindeki son
                                                         SN saniyenin olaylarını kanala gönderir

Ortam değişkenleri:
  DISCORD_TOKEN        bot token'ı (zorunlu; kaynağa yazılmaz)
  DISCORD_GUILD_ID     sunucu kimliği      (varsayılan: Mangus sunucusu)
  DISCORD_CHANNEL_ID   olay kanalı kimliği (varsayılan: Mangus olay kanalı)
  DISCORD_USER_ID      check-user için varsayılan kullanıcı
  MANGUS_ROOT          depo kökü (varsayılan: bu betiğin üst dizini)

Dosyalar sunucuyla aynı biçimdedir: UTF-8; olay günlüğü '|' ile ayrılmış
"YYYY/AA/GG SS:DD:ss|oyun yılı|ay|gün|saat|ileti" satırları (src/data.c).
"""
import argparse
import os
import sys
from datetime import datetime
from pathlib import Path

import discord

ROOT = Path(os.environ.get("MANGUS_ROOT") or Path(__file__).resolve().parents[1])
EVENTS_FILE = ROOT / "log" / "events" / "events"
USERS_FILE = ROOT / "data" / "discord_users"
DEFAULT_GUILD_ID = 1002158648744742963
DEFAULT_CHANNEL_ID = 1003974452989673492
DEFAULT_USER_ID = 656956609930723349
EVENT_TIME_FORMAT = "%Y/%m/%d %H:%M:%S"
TAIL_BYTES = 256 * 1024      # büyüyen olay günlüğünün yalnızca sonu okunur


def env_int(name, default):
    value = os.environ.get(name, "").strip()
    return int(value) if value else default


def make_client(members=False):
    intents = discord.Intents.default()
    intents.typing = False
    intents.presences = False
    intents.members = members
    return discord.Client(intents=intents)


def run(client, token, job):
    """Bot hazır olunca ``job`` eş yordamını çalıştırıp bağlantıyı kapatır."""
    @client.event
    async def on_ready():
        try:
            await job(client)
        finally:
            await client.close()

    client.run(token)


def recent_events(path, window_seconds, now=None):
    """Olay günlüğündeki son ``window_seconds`` saniyeye ait iletiler (dosya sırasıyla)."""
    now = now or datetime.now()
    if not path.is_file():
        return []
    size = path.stat().st_size
    with open(path, "rb") as fh:
        if size > TAIL_BYTES:
            fh.seek(size - TAIL_BYTES)
            fh.readline()                      # yarım satırı at
        data = fh.read().decode("utf-8", "replace")
    messages = []
    for line in data.splitlines():
        fields = line.split("|", 5)
        if len(fields) < 6:
            continue
        try:
            stamp = datetime.strptime(fields[0], EVENT_TIME_FORMAT)
        except ValueError:
            continue
        if (now - stamp).total_seconds() <= window_seconds:
            messages.append(fields[5])
    return messages


def cmd_check_user(args):
    user_id = args.user_id or env_int("DISCORD_USER_ID", DEFAULT_USER_ID)

    async def job(client):
        user = await client.fetch_user(user_id)
        print(user.name)

    run(make_client(), args.token, job)


def cmd_members(args):
    guild_id = env_int("DISCORD_GUILD_ID", DEFAULT_GUILD_ID)

    async def job(client):
        guild = client.get_guild(guild_id)
        if guild is None:
            sys.exit("Sunucu bulunamadı: %d (bot üye mi, DISCORD_GUILD_ID doğru mu?)" % guild_id)
        USERS_FILE.parent.mkdir(parents=True, exist_ok=True)
        with open(USERS_FILE, "w", encoding="utf-8") as fh:
            for member in guild.members:
                fh.write("%d\n" % member.id)
        print("%d üye -> %s" % (len(guild.members), USERS_FILE))

    run(make_client(members=True), args.token, job)


def cmd_feed_events(args):
    channel_id = env_int("DISCORD_CHANNEL_ID", DEFAULT_CHANNEL_ID)
    messages = recent_events(EVENTS_FILE, args.window)
    if not messages:
        return

    async def job(client):
        channel = client.get_channel(channel_id)
        if channel is None:
            sys.exit("Kanal bulunamadı: %d (DISCORD_CHANNEL_ID doğru mu?)" % channel_id)
        for message in messages:
            await channel.send(message)

    run(make_client(), args.token, job)


def main(argv=None):
    parser = argparse.ArgumentParser(description="Mangus Discord araçları")
    sub = parser.add_subparsers(dest="command", required=True)

    p = sub.add_parser("check-user", help="kullanıcı adını yazdır")
    p.add_argument("user_id", nargs="?", type=int)
    p.set_defaults(func=cmd_check_user)

    p = sub.add_parser("members", help="sunucu üyelerinin kimliklerini data/discord_users'a yaz")
    p.set_defaults(func=cmd_members)

    p = sub.add_parser("feed-events", help="son olayları kanala gönder")
    p.add_argument("--window", type=int, default=300, help="kaç saniye geriye bakılır (varsayılan 300)")
    p.set_defaults(func=cmd_feed_events)

    args = parser.parse_args(argv)
    args.token = os.environ.get("DISCORD_TOKEN", "").strip()
    if not args.token:
        sys.exit("DISCORD_TOKEN ortam değişkeni tanımlı değil.")
    args.func(args)


if __name__ == "__main__":
    main()
