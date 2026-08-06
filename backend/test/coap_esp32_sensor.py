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
                "moisture1": 450,
                "moisture2": 460,
                "moisture3": 470,
                "moisture4": 480,
                "moisture5": 490,
                "moisture6": 500,
                "flowrate": 1.5,
                "total_litres": 10.0,
                "distance_cm": 15.0,
            }
        ).encode("utf-8")

        request = Message(
            code=Code.PUT,
            payload=payload,
            uri="coap://127.0.0.1:8683/coap/hydroponics/plant",
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
