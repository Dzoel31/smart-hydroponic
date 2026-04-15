from fastapi import WebSocket
from collections import defaultdict
import asyncio


class RoomConnectionManager:
    def __init__(self):
        self.rooms = defaultdict(lambda: defaultdict(dict))
        self.lock = asyncio.Lock()

    async def connect(self, room: str, role: str, client_id: str, websocket: WebSocket):
        # await websocket.accept()
        async with self.lock:
            self.rooms[room][role][client_id] = websocket

    async def disconnect(self, room: str, role: str, client_id: str):
        async with self.lock:
            self.rooms.get(room, {}).get(role, {}).pop(client_id, None)

            if not self.rooms[room][role]:
                del self.rooms[room][role]

            if not self.rooms[room]:
                del self.rooms[room]

    async def send_to_room(self, room: str, role: str, message: dict):
        dead_clients = []

        for client_id, ws in self.rooms.get(room, {}).get(role, {}).items():
            try:
                await ws.send_json(message)
            except Exception:
                dead_clients.append(client_id)

        for client_id in dead_clients:
            await self.disconnect(room, role, client_id)

    async def send_to_client(self, room: str, role: str, client_id: str, message: dict):
        ws = self.rooms.get(room, {}).get(role, {}).get(client_id)
        if ws:
            await ws.send_json(message)


manager = RoomConnectionManager()
