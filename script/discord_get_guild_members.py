import asyncio
import discord

intents = discord.Intents.default()
intents.typing = False
intents.presences = False
intents.members = True
client = discord.Client(intents=intents)

@client.event
async def on_ready():
    await client.wait_until_ready()
    guild = client.get_guild(1002158648744742963)
    with open('/home/ozgur/mud/mangus/data/discord_users',encoding='iso8859-9', mode='w') as discord_file:
        for member in guild.members:
            discord_file.write(str(member.id)+"\n")
    await client.close()

client.run("MTAwMzc4MDc3OTYzMDkyNzg3Mg.GYsVEl.Wy9NOVT2W374XP_3I6WVE9K-ucky7sCT9h4Xzk")
