import discord
import csv
from discord.ext import commands

with open('../log/events/events') as csv_file:
    csv_reader = csv.reader(csv_file, delimiter='|')

client = discord.Client()

@client.event
async def on_ready():  #  Called when internal cache is loaded

    channel = client.get_channel(1003974452989673492) #  Olaylar kanali
    await channel.send("Bot test: 4") #  Sends message to channel
    await client.close()


client.run("MTAwMzc4MDc3OTYzMDkyNzg3Mg.GYsVEl.Wy9NOVT2W374XP_3I6WVE9K-ucky7sCT9h4Xzk")  # Starts up the bot