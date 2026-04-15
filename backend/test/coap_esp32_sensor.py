import asyncio
from aiocoap import Context, Message, Code
import json


async def main():
    context = await Context.create_client_context()

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
        code=Code.PUT, payload=payload, uri="coap://localhost/coap/hydroponics/sensor"
    )
    response = await context.request(request).response
    print(f"Response Code: {response.code}")
    print(f"Response Payload: {response.payload.decode('utf-8')}")


if __name__ == "__main__":
    asyncio.run(main())
