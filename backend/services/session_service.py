import secrets

from datetime import datetime, timedelta, timezone
from sqlalchemy import text
from sqlalchemy.ext.asyncio import AsyncSession


class SessionService:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def create_session(self, userid: str):

        session_id = secrets.token_urlsafe(64)

        now = datetime.now(timezone.utc)

        expires_at = now + timedelta(hours=1)

        stmt = text("""
            INSERT INTO user_sessions
            (
                session_id,
                userid,
                created_at,
                expires_at
            )
            VALUES
            (
                :session_id,
                :userid,
                :created_at,
                :expires_at
            )
        """)

        await self.session.execute(
            stmt,
            {
                "session_id": session_id,
                "userid": userid,
                "created_at": now,
                "expires_at": expires_at,
            }
        )

        await self.session.commit()

        return session_id

    async def get_session(self, session_id: str):

        stmt = text("""
            SELECT *
            FROM user_sessions
            WHERE session_id = :session_id
        """)

        result = await self.session.execute(
            stmt,
            {"session_id": session_id}
        )

        return result.mappings().first()

    async def delete_session(self, session_id: str):

        stmt = text("""
            DELETE FROM user_sessions
            WHERE session_id = :session_id
        """)

        await self.session.execute(
            stmt,
            {"session_id": session_id}
        )

        await self.session.commit()