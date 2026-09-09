import os
import asyncio
import discord
import csv
from datetime import datetime

client = discord.Client()

@client.event
async def on_ready():
    await client.wait_until_ready()
    user = await client.fetch_user(656956609930723349)
    print(user.name)
    await client.close()

client.run(os.environ["DISCORD_TOKEN"])
