import asyncio
from aiocoap import Context, Message, Code
import json


async def main():
    context = await Context.create_client_context()

    payload = json.dumps(
        {
            "temperature_atas": 22,
            "temperature_bawah": 23,
            "humidity_atas": 55,
            "humidity_bawah": 60,
            "light_intensity_atas": 800,
            "light_intensity_bawah": 750,
        }
    ).encode("utf-8")

    request = Message(
        code=Code.PUT,
        payload=payload,
        uri="coap://localhost/coap/hydroponics/environment",
    )
    response = await context.request(request).response
    print(f"Response Code: {response.code}")
    print(f"Response Payload: {response.payload.decode('utf-8')}")


if __name__ == "__main__":
    asyncio.run(main())
