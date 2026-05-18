from datetime import datetime
import uuid

from config.db import Base
from sqlalchemy import Boolean, DateTime, Double, String, Text, func
from sqlalchemy.dialects.postgresql import UUID
from sqlalchemy.orm import Mapped, mapped_column


class PlantNutritionProfile(Base):
    __tablename__ = "plant_nutrition_profiles"

    nutrition_id: Mapped[uuid.UUID] = mapped_column(
        UUID(as_uuid=True),
        primary_key=True,
        default=uuid.uuid4,
    )
    plant_name: Mapped[str] = mapped_column(String(120), unique=True, index=True)
    is_active: Mapped[bool] = mapped_column(Boolean, nullable=False, default=False)
    moisture_min: Mapped[float] = mapped_column(Double, nullable=False, default=0.0)
    moisture_max: Mapped[float] = mapped_column(Double, nullable=False, default=0.0)
    ph_min: Mapped[float] = mapped_column(Double, nullable=False, default=0.0)
    ph_max: Mapped[float] = mapped_column(Double, nullable=False, default=0.0)
    tds_min: Mapped[float] = mapped_column(Double, nullable=False, default=0.0)
    tds_max: Mapped[float] = mapped_column(Double, nullable=False, default=0.0)
    temperature_min: Mapped[float] = mapped_column(Double, nullable=False, default=0.0)
    temperature_max: Mapped[float] = mapped_column(Double, nullable=False, default=0.0)
    humidity_min: Mapped[float] = mapped_column(Double, nullable=False, default=0.0)
    humidity_max: Mapped[float] = mapped_column(Double, nullable=False, default=0.0)
    notes: Mapped[str | None] = mapped_column(Text, nullable=True)
    created_at: Mapped[datetime] = mapped_column(
        DateTime(timezone=True),
        server_default=func.now(),
        nullable=False,
    )
    updated_at: Mapped[datetime] = mapped_column(
        DateTime(timezone=True),
        server_default=func.now(),
        onupdate=func.now(),
        nullable=False,
    )