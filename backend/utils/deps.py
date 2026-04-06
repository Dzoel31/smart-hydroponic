from typing import AsyncGenerator
from sqlalchemy.ext.asyncio import AsyncSession
from config.db import Session
from fastapi import Depends, HTTPException, status
from fastapi.security import HTTPBearer, HTTPAuthorizationCredentials
from schemas.user import TokenPayload, UserOut
from services.user_service import UserService
from contextlib import asynccontextmanager
from authlib.jose.errors import JoseError

bearer_scheme = HTTPBearer()


async def get_session() -> AsyncGenerator[AsyncSession, None]:
    """Provide a SQLAlchemy async session (expire_on_commit=False)"""
    async with Session() as session:
        yield session


async def get_current_user(
    credentials: HTTPAuthorizationCredentials = Depends(bearer_scheme),
    session: AsyncSession = Depends(get_session),
):
    service = UserService(session)
    try:
        token = credentials.credentials
        payload = service.verify_token(token)
        payload = TokenPayload.model_validate(payload)

        if payload.id is None:
            raise HTTPException(
                status_code=status.HTTP_401_UNAUTHORIZED,
                detail="Invalid token payload",
            )

    except JoseError:
        raise HTTPException(
            status_code=status.HTTP_401_UNAUTHORIZED,
            detail="Could not validate credentials",
        )

    user = await service.get_user_by_id(payload.id)
    if not user:
        raise HTTPException(
            status_code=status.HTTP_404_NOT_FOUND,
            detail="User not found",
        )
    return UserOut.model_validate(user)


# Untuk penggunaan di luar konteks Depends e.g WebSocket
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
