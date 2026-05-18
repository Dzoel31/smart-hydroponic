from typing import Any
from uuid import UUID

from sqlalchemy import func, select, update
from sqlalchemy.ext.asyncio import AsyncSession

from models.nutrition import PlantNutritionProfile
from schemas.hydroponic import MetaData, ResponseList
from schemas.nutrition import (
    PlantNutritionProfileCreate,
    PlantNutritionProfileOut,
    PlantNutritionProfileUpdate,
)


class NutritionService:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def create_profile(
        self, profile_data: PlantNutritionProfileCreate
    ) -> PlantNutritionProfileOut:
        has_active_profile = await self.get_active_profile()
        profile = PlantNutritionProfile(
            **profile_data.model_dump(),
            is_active=has_active_profile is None,
        )
        self.session.add(profile)
        await self.session.commit()
        await self.session.refresh(profile)
        return PlantNutritionProfileOut.model_validate(profile)

    async def get_profiles(
        self,
        page: int = 1,
        limit: int = 25,
    ) -> ResponseList[PlantNutritionProfileOut]:
        offset = (page - 1) * limit

        data_stmt = (
            select(PlantNutritionProfile)
            .order_by(PlantNutritionProfile.is_active.desc(), PlantNutritionProfile.plant_name.asc())
            .limit(limit)
            .offset(offset)
        )
        count_stmt = select(func.count()).select_from(PlantNutritionProfile)

        data_result = await self.session.execute(data_stmt)
        total_rows = await self.session.scalar(count_stmt)

        return ResponseList(
            meta=MetaData(total_rows=total_rows, limit=limit, offset=offset),
            data=[PlantNutritionProfileOut.model_validate(row) for row in data_result.scalars().all()],
        )

    async def get_active_profile(self) -> PlantNutritionProfile | None:
        stmt = select(PlantNutritionProfile).where(PlantNutritionProfile.is_active.is_(True))
        result = await self.session.execute(stmt)
        return result.scalars().first()

    async def get_profile_by_id(self, nutrition_id: UUID | str) -> PlantNutritionProfile | None:
        stmt = select(PlantNutritionProfile).where(
            PlantNutritionProfile.nutrition_id == nutrition_id
        )
        result = await self.session.execute(stmt)
        return result.scalars().first()

    async def update_profile(
        self,
        nutrition_id: UUID | str,
        profile_update: PlantNutritionProfileUpdate,
    ) -> PlantNutritionProfileOut | None:
        profile = await self.get_profile_by_id(nutrition_id)
        if profile is None:
            return None

        update_data = profile_update.model_dump(exclude_unset=True)
        if not update_data:
            return PlantNutritionProfileOut.model_validate(profile)

        merged_data: dict[str, Any] = {
            "plant_name": profile.plant_name,
            "moisture_min": profile.moisture_min,
            "moisture_max": profile.moisture_max,
            "ph_min": profile.ph_min,
            "ph_max": profile.ph_max,
            "tds_min": profile.tds_min,
            "tds_max": profile.tds_max,
            "temperature_min": profile.temperature_min,
            "temperature_max": profile.temperature_max,
            "humidity_min": profile.humidity_min,
            "humidity_max": profile.humidity_max,
            "notes": profile.notes,
        }
        merged_data.update(update_data)

        validated = PlantNutritionProfileCreate.model_validate(merged_data)
        for field, value in validated.model_dump().items():
            setattr(profile, field, value)

        await self.session.commit()
        await self.session.refresh(profile)
        return PlantNutritionProfileOut.model_validate(profile)

    async def set_active_profile(self, nutrition_id: UUID | str) -> PlantNutritionProfileOut | None:
        profile = await self.get_profile_by_id(nutrition_id)
        if profile is None:
            return None

        await self.session.execute(
            update(PlantNutritionProfile).values(is_active=False)
        )
        profile.is_active = True
        await self.session.commit()
        await self.session.refresh(profile)
        return PlantNutritionProfileOut.model_validate(profile)

    async def delete_profile(self, nutrition_id: UUID | str) -> bool:
        profile = await self.get_profile_by_id(nutrition_id)
        if profile is None:
            return False

        await self.session.delete(profile)
        await self.session.commit()
        return True
