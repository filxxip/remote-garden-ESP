import asyncio
import os
from dotenv import load_dotenv
from bleak import BleakClient, BleakScanner

SSID_UUID = "0000ff01-0000-1000-8000-00805f9b34fb"
PASS_UUID = "0000ff02-0000-1000-8000-00805f9b34fb"
USER_KEY_UUID = "0000ff03-0000-1000-8000-00805f9b34fb" 

load_dotenv()

async def main():
    ssid_value = os.getenv("SSID")
    pass_value = os.getenv("PASSWORD")
    user_key_value = os.getenv("USER_KEY")

    if not ssid_value or not pass_value or not user_key_value:
        print("ERROR: SSID, PASSWORD or USER_KEY not set in .env file.")
        return

    print("Scanning for BLE devices...")
    devices = await BleakScanner.discover()

    for d in devices:
        print(f"Found: {d.name} [{d.address}]")
    
    esp = next((d for d in devices if "ESP" in (d.name or "")), None)

    if not esp:
        print("ESP32 not found.")
        return

    print(f"Connecting to {esp.name} [{esp.address}]...")
    async with BleakClient(esp.address) as client:
        print("Connected!")

        print("Sending SSID...")
        await client.write_gatt_char(SSID_UUID, ssid_value.encode("utf-8"))
        await asyncio.sleep(1.0)

        print("Sending Password...")
        await client.write_gatt_char(PASS_UUID, pass_value.encode("utf-8"))
        await asyncio.sleep(1.0)

        print("Sending User Key...")
        await client.write_gatt_char(USER_KEY_UUID, user_key_value.encode("utf-8"))
        await asyncio.sleep(1.0)

        print("Done. Disconnecting.")

if __name__ == "__main__":
    asyncio.run(main())
