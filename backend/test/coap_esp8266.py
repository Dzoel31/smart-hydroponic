import asyncio
from aiocoap import Context, Message, Code
import json


async def main():
    context = await Context.create_client_context()

    payload = json.dumps(
        {
            "automation_status": True,
            "pump_status": True,
            "light_status": False,
        }
    ).encode("utf-8")

    request = Message(
        code=Code.PUT, payload=payload, uri="coap://localhost/coap/hydroponics/actuator"
    )
    response = await context.request(request).response
    print(f"Response Code: {response.code}")
    print(f"Response Payload: {response.payload.decode('utf-8')}")


if __name__ == "__main__":
    asyncio.run(main())
