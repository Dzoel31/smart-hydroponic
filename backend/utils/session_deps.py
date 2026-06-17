from typing import AsyncGenerator
from sqlalchemy.ext.asyncio import AsyncSession
from config.db import Session
from fastapi import Cookie, Depends, HTTPException, status
from schemas.user import UserOut
from services.user_service import UserService
from services.session_service import SessionService
from contextlib import asynccontextmanager


async def get_session() -> AsyncGenerator[AsyncSession, None]:
    """Provide a SQLAlchemy async session (expire_on_commit=False)"""
    async with Session() as session:
        yield session

async def get_current_user_session(
    session_id: str | None = Cookie(default=None),
    session: AsyncSession = Depends(get_session)
) -> UserOut:

    if not session_id:
        raise HTTPException(
            status_code=401,
            detail="Not authenticated"
        )

    session_service = SessionService(session)
    session_data = await session_service.get_session(session_id)

    if not session_data:
        raise HTTPException(
            status_code=401,
            detail="Invalid session"
        )

    user_service = UserService(session)

    user = await user_service.get_user_by_id(
        str(session_data["userid"])
    )

    if not user:
        raise HTTPException(
            status_code=404,
            detail="User not found"
        )

    return UserOut.model_validate(user)

async def get_optional_current_user(
    session_id: str | None = Cookie(default=None),
    session: AsyncSession = Depends(get_session),
) -> UserOut | None:

    if not session_id:
        return None
 
    session_service = SessionService(session)
    user_session = await session_service.get_valid_session(session_id)
 
    if not user_session:
        return None
 
    user_service = UserService(session)
    user = await user_service.get_user_by_id(str(user_session["userid"]))
 
    if not user:
        return None
 
    return UserOut.model_validate(user)

@asynccontextmanager
async def get_db_session() -> AsyncGenerator[AsyncSession, None]:
    """Context manager to provide a SQLAlchemy async session."""
    async with Session() as session:
        try:
            yield session
        finally:
            await session.close()

def require_role(current_user: UserOut | None, allowed_roles: set[str]) -> None:
    if current_user is None or current_user.role not in allowed_roles:
        raise HTTPException(status_code=403, detail="Permission denied")