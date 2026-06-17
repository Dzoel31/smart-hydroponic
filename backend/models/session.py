from sqlalchemy.orm import Mapped, mapped_column
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy import String, DateTime, Boolean, ForeignKey, func
from datetime import datetime
import uuid

from config.db import Base


class UserSession(Base):
    __tablename__ = "user_sessions"

    session_id: Mapped[str] = mapped_column(
        String(255), primary_key=True
    )
    userid: Mapped[uuid.UUID] = mapped_column(
        UUID(as_uuid=True),
        ForeignKey("user_data.userid", ondelete="CASCADE"),
        nullable=False,
    )
    username: Mapped[str] = mapped_column(
        String(50), nullable=False, default=""
    )
    role: Mapped[str] = mapped_column(
        String(20), nullable=False, default="admin"
    )
    created_at: Mapped[datetime] = mapped_column(
        DateTime(timezone=True),
        server_default=func.now(),
        nullable=False,
    )
    expires_at: Mapped[datetime] = mapped_column(
        DateTime(timezone=True), nullable=False
    )
    is_active: Mapped[bool] = mapped_column(
        Boolean, nullable=False, default=True
    )