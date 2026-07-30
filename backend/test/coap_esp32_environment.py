import asyncio
from aiocoap import Context, Message, Code
import json

import sys

if sys.platform == "win32":
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())


async def main():
    context = await Context.create_client_context()

    while True:
        payload = json.dumps(
            {
                "temperature_atas": 26.5,
                "temperature_bawah": 25.8,
                "humidity_atas": 60.5,
                "humidity_bawah": 62.0,
                "ph": 6.2,
                "tds": 800.0,
            }
        ).encode("utf-8")

        request = Message(
            code=Code.PUT,
            payload=payload,
            uri="coap://127.0.0.1:8683/coap/hydroponics/environment",
        )
        try:
            response = await context.request(request).response
            print(f"Response Code: {response.code}")
            print(f"Response Payload: {response.payload.decode('utf-8')}")
        except Exception as e:
            print(f"Error: {e}")

        await asyncio.sleep(30)


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nDihentikan oleh pengguna (CTRL+C).")
