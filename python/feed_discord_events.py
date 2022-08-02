from discord.ext import commands
import asyncio
import csv
from datetime import datetime

time = datetime.now

bot = commands.Bot(command_prefix='!')

async def timer():
    await bot.wait_until_ready()
    channel = bot.get_channel(1003974452989673492) # replace with channel ID that you want to send to
    msg_sent = False

    with open('../log/events/events',encoding='iso8859-9') as csv_file:
        csv_reader = csv.reader(csv_file, delimiter='|')
        for row in csv_reader:
            tarih = datetime.strptime(row[0], '%Y/%m/%d %H:%M:%S')
            oyun_yili = row[1]
            oyun_ayi = row[2]
            oyun_gunu = row[3]
            oyun_saati = row[4]
            mesaj = row[5]
            #print((now-tarih).total_seconds())
            if ((datetime.now()-tarih).total_seconds()<=60):
                await channel.send(mesaj)

    await asyncio.sleep(60)

bot.loop.create_task(timer())
bot.run("MTAwMzc4MDc3OTYzMDkyNzg3Mg.GYsVEl.Wy9NOVT2W374XP_3I6WVE9K-ucky7sCT9h4Xzk")