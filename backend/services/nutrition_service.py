import uuid
from uuid import UUID

from sqlalchemy import text
from sqlalchemy.ext.asyncio import AsyncSession

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
        has_active = await self.get_active_profile()
        is_active = has_active is None

        data_dict = profile_data.model_dump()
        data_dict["nutrition_id"] = uuid.uuid4()
        data_dict["is_active"] = is_active

        columns = ", ".join(f"{k}" for k in data_dict.keys())
        placeholders = ", ".join(f":{k}" for k in data_dict.keys())

        stmt = text(f"""
            INSERT INTO plant_nutrition_profiles ({columns})
            VALUES ({placeholders})
            RETURNING *
        """)

        result = await self.session.execute(stmt, data_dict)
        await self.session.commit()
        record = result.mappings().first()
        return PlantNutritionProfileOut.model_validate(record)

    async def get_profiles(
        self,
        page: int = 1,
        limit: int = 25,
    ) -> ResponseList[PlantNutritionProfileOut]:
        offset = (page - 1) * limit

        stmt_data = text("""
            SELECT * FROM plant_nutrition_profiles
            ORDER BY is_active DESC, plant_name ASC
            LIMIT :limit OFFSET :offset
        """)
        stmt_count = text("SELECT COUNT(*) FROM plant_nutrition_profiles")

        data_result = await self.session.execute(
            stmt_data, {"limit": limit, "offset": offset}
        )
        total_rows = await self.session.scalar(stmt_count)

        return ResponseList(
            meta=MetaData(total_rows=total_rows, limit=limit, offset=offset),
            data=[
                PlantNutritionProfileOut.model_validate(row)
                for row in data_result.mappings().all()
            ],
        )

    async def get_active_profile(self) -> PlantNutritionProfileOut | None:
        stmt = text("SELECT * FROM plant_nutrition_profiles WHERE is_active = true")
        result = await self.session.execute(stmt)
        record = result.mappings().first()
        if record:
            return PlantNutritionProfileOut.model_validate(record)
        return None

    async def get_profile_by_id(
        self, nutrition_id: UUID | str
    ) -> PlantNutritionProfileOut | None:
        stmt = text("SELECT * FROM plant_nutrition_profiles WHERE nutrition_id = :id")
        result = await self.session.execute(stmt, {"id": nutrition_id})
        record = result.mappings().first()
        if record:
            return PlantNutritionProfileOut.model_validate(record)
        return None

    async def update_profile(
        self,
        nutrition_id: UUID | str,
        profile_update: PlantNutritionProfileUpdate,
    ) -> PlantNutritionProfileOut | None:
        profile = await self.get_profile_by_id(nutrition_id)
        if not profile:
            return None

        update_data = profile_update.model_dump(exclude_unset=True)
        if not update_data:
            return profile

        set_clause = ", ".join(f"{k} = :{k}" for k in update_data.keys())
        stmt = text(f"""
            UPDATE plant_nutrition_profiles
            SET {set_clause}, updated_at = NOW()
            WHERE nutrition_id = :id
            RETURNING *
        """)

        params = {**update_data, "id": nutrition_id}
        result = await self.session.execute(stmt, params)
        await self.session.commit()
        record = result.mappings().first()
        return PlantNutritionProfileOut.model_validate(record)

    async def set_active_profile(
        self, nutrition_id: UUID | str
    ) -> PlantNutritionProfileOut | None:
        profile = await self.get_profile_by_id(nutrition_id)
        if not profile:
            return None

        await self.session.execute(
            text("UPDATE plant_nutrition_profiles SET is_active = false")
        )

        stmt = text("""
            UPDATE plant_nutrition_profiles 
            SET is_active = true 
            WHERE nutrition_id = :id 
            RETURNING *
        """)
        result = await self.session.execute(stmt, {"id": nutrition_id})
        await self.session.commit()

        record = result.mappings().first()
        return PlantNutritionProfileOut.model_validate(record)

    async def delete_profile(self, nutrition_id: UUID | str) -> bool:
        profile = await self.get_profile_by_id(nutrition_id)
        if not profile:
            return False

        stmt = text("DELETE FROM plant_nutrition_profiles WHERE nutrition_id = :id")
        await self.session.execute(stmt, {"id": nutrition_id})
        await self.session.commit()
        return True
