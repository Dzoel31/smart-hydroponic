from fastapi import Cookie, Depends, HTTPException
from sqlalchemy.ext.asyncio import AsyncSession

from utils.deps import get_session
from services.user_service import UserService
from services.session_service import SessionService
from schemas.user import UserOut


async def get_current_user_session(
    session_id: str | None = Cookie(default=None),
    db: AsyncSession = Depends(get_session)
):

    if not session_id:
        raise HTTPException(
            status_code=401,
            detail="Not authenticated"
        )

    session_service = SessionService(db)

    session_data = await session_service.get_session(session_id)

    if not session_data:
        raise HTTPException(
            status_code=401,
            detail="Invalid session"
        )

    user_service = UserService(db)

    user = await user_service.get_user_by_id(
        str(session_data["userid"])
    )

    if not user:
        raise HTTPException(
            status_code=404,
            detail="User not found"
        )

    return UserOut.model_validate(user)