import asyncio
from aiocoap import Context, Message, Code
import sys

# Event loop policy wajib untuk aiocoap di Windows
if sys.platform == "win32":
    asyncio.set_event_loop_policy(asyncio.WindowsSelectorEventLoopPolicy())


import json

# State lokal dari aktuator
state = {
    "automation_status": True,
    "pump_status": True,
    "light_status": False,
}


async def observe_task(context):
    print("Starting CoAP Observer for Actuator...")
    request = Message(
        code=Code.GET, uri="coap://127.0.0.1:8683/coap/hydroponics/actuator", observe=0
    )
    try:
        pr = context.request(request)
        response = await pr.response
        print(f"[Observer] Initial State: {response.payload.decode('utf-8')}")

        if pr.observation is None:
            print("[Observer] Server tidak mendukung Observe.")
            return

        async for notification in pr.observation:
            print(
                f"[Observer] Update received dari Server: {notification.payload.decode('utf-8')}"
            )
            try:
                # Update state lokal sesuai perintah server
                new_state = json.loads(notification.payload.decode("utf-8"))
                state.update(new_state)
            except Exception:
                pass
    except asyncio.CancelledError:
        pass
    except Exception as e:
        print(f"[Observer] Error: {e}")


async def report_task(context):
    print("Starting CoAP Reporter for Actuator...")
    while True:
        try:
            payload = json.dumps(state).encode("utf-8")
            request = Message(
                code=Code.PUT,
                payload=payload,
                uri="coap://127.0.0.1:8683/coap/hydroponics/actuator",
            )
            response = await context.request(request).response
            print(
                f"[Reporter] Melaporkan state lokal ke Server. (Code: {response.code})"
            )
        except asyncio.CancelledError:
            break
        except Exception as e:
            print(f"[Reporter] Error: {e}")

        await asyncio.sleep(30)


async def main():
    context = await Context.create_client_context()

    try:
        # Jalankan observer dan reporter secara bersamaan
        await asyncio.gather(observe_task(context), report_task(context))
    except asyncio.CancelledError:
        print("\nDihentikan oleh pengguna.")
    finally:
        await context.shutdown()


if __name__ == "__main__":
    try:
        asyncio.run(main())
    except KeyboardInterrupt:
        print("\nDihentikan oleh pengguna (CTRL+C).")
