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

client.run("MTAwMzc4MDc3OTYzMDkyNzg3Mg.GYsVEl.Wy9NOVT2W374XP_3I6WVE9K-ucky7sCT9h4Xzk")
