from typing import TYPE_CHECKING
from sqlalchemy.orm import Mapped, mapped_column, relationship
from sqlalchemy import ForeignKey, Text
from sqlalchemy.dialects.postgresql import UUID
import uuid
from config.db import Base

if TYPE_CHECKING:
    from models.user import User


class Log(Base):
    __tablename__ = "logs"

    logid: Mapped[uuid.UUID] = mapped_column(
        UUID(as_uuid=True), primary_key=True, default=uuid.uuid7
    )
    userid: Mapped[uuid.UUID] = mapped_column(
        UUID(as_uuid=True),
        ForeignKey("user_data.userid", ondelete="CASCADE"),
    )
    description: Mapped[str] = mapped_column(Text)
    user: Mapped["User"] = relationship("User", back_populates="logs")
