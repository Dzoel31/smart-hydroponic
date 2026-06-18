import secrets
import logging
from datetime import datetime, timedelta, timezone
from sqlalchemy import text
from sqlalchemy.ext.asyncio import AsyncSession

logger = logging.getLogger(__name__)

SESSION_EXPIRE_MINUTES = 1


class SessionService:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def create_session(self, user: dict) -> str:
        await self.session.execute(
            text("""
                UPDATE user_sessions
                SET is_active = FALSE
                WHERE userid = :userid AND is_active = TRUE
            """),
            {"userid": user["userid"]},
        )

        session_id = secrets.token_urlsafe(64)
        now = datetime.now(timezone.utc)
        expires_at = now + timedelta(minutes=SESSION_EXPIRE_MINUTES)

        await self.session.execute(
            text("""
                INSERT INTO user_sessions
                    (session_id, userid, username, role, created_at, expires_at, is_active)
                VALUES
                    (:session_id, :userid, :username, :role, :created_at, :expires_at, TRUE)
            """),
            {
                "session_id": session_id,
                "userid":     user["userid"],
                "username":   user["username"],
                "role":       user["role"],
                "created_at": now,
                "expires_at": expires_at,
            },
        )
        await self.session.commit()
        logger.info("Session created for user: %s", user["username"])
        return session_id

    async def get_valid_session(self, session_id: str) -> dict | None:
        """
        Cek session masih aktif dan belum expired.
        Dipanggil di setiap request oleh get_current_user di deps.py.
        """
        now = datetime.now(timezone.utc)
        result = await self.session.execute(
            text("""
                SELECT * FROM user_sessions
                WHERE session_id = :session_id
                  AND is_active  = TRUE
                  AND expires_at > :now
            """),
            {"session_id": session_id, "now": now},
        )
        row = result.mappings().first()
        return dict(row) if row else None

    async def invalidate_session(self, session_id: str) -> None:
        """
        Logout: tandai session tidak aktif (tidak dihapus,
        supaya replay attack bisa dideteksi).
        """
        await self.session.execute(
            text("""
                UPDATE user_sessions
                SET is_active = FALSE
                WHERE session_id = :session_id
            """),
            {"session_id": session_id},
        )
        await self.session.commit()
        logger.info("Session invalidated: %s...", session_id[:12])

    # Tetap ada untuk backward compatibility
    async def get_session(self, session_id: str) -> dict | None:
        result = await self.session.execute(
            text("SELECT * FROM user_sessions WHERE session_id = :session_id"),
            {"session_id": session_id},
        )
        row = result.mappings().first()
        return dict(row) if row else None

    async def delete_session(self, session_id: str) -> None:
        await self.session.execute(
            text("DELETE FROM user_sessions WHERE session_id = :session_id"),
            {"session_id": session_id},
        )
        await self.session.commit()