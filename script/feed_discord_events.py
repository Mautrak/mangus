import os
import asyncio
import discord
import csv
from datetime import datetime

client = discord.Client()

@client.event
async def on_ready():
    await client.wait_until_ready()
    channel = client.get_channel(1003974452989673492)
    with open('/home/ozgur/mud/mangus/log/events/events',encoding='iso8859-9') as csv_file:
        csv_reader = csv.reader(csv_file, delimiter='|')
        for row in csv_reader:
            tarih = datetime.strptime(row[0], '%Y/%m/%d %H:%M:%S')
            oyun_yili = row[1]
            oyun_ayi = row[2]
            oyun_gunu = row[3]
            oyun_saati = row[4]
            mesaj = row[5]
            now = datetime.now()
            #print((now-tarih).total_seconds())
            if ((now-tarih).total_seconds()<=300):
                await channel.send(mesaj)
    await client.close()

client.run(os.environ["DISCORD_TOKEN"])
