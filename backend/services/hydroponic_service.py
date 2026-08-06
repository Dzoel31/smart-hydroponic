from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy import text
from typing import Any
from schemas.hydroponic import (
    HydroponicDataPlant,
    HydroponicDataEnvironment,
    HydroponicDataActuator,
    HydroponicIn,
    HydroponicOut,
    MetaData,
    ResponseList,
)
from utils.converter import get_uuidv7_from_timestamp

SENSOR_FIELDS = set(HydroponicDataPlant.model_fields.keys())
ENVIRONMENT_FIELDS = set(HydroponicDataEnvironment.model_fields.keys())
ACTUATOR_FIELDS = set(HydroponicDataActuator.model_fields.keys())

GROUPS = {
    "sensor": SENSOR_FIELDS,
    "environment": ENVIRONMENT_FIELDS,
    "actuator": ACTUATOR_FIELDS,
}


class HydroponicService:
    def __init__(self, session: AsyncSession):
        self.session = session

    async def add_data(self, hydroponic_data: HydroponicIn) -> HydroponicOut:
        from uuid import uuid7

        data_dict = hydroponic_data.model_dump()
        if "dataid" not in data_dict:
            data_dict["dataid"] = uuid7()

        columns = ", ".join(f'"{k}"' for k in data_dict.keys())
        placeholders = ", ".join(f":{k}" for k in data_dict.keys())

        stmt = text(f"""
            INSERT INTO hydroponic_data ({columns})
            VALUES ({placeholders})
            RETURNING *
        """)

        result = await self.session.execute(stmt, data_dict)
        await self.session.commit()
        record = result.mappings().first()
        return HydroponicOut.model_validate(record)

    async def get_all_data(
        self,
        page: int = 1,
        limit: int = 25,
        start_date: str | None = None,
        end_date: str | None = None,
    ) -> ResponseList[HydroponicOut]:
        filters = []
        params: dict[str, Any] = {"limit": limit, "offset": (page - 1) * limit}

        _start_date = get_uuidv7_from_timestamp(start_date) if start_date else None
        _end_date = (
            get_uuidv7_from_timestamp(end_date, is_end=True) if end_date else None
        )

        if _start_date and _end_date:
            filters.append('"dataid" >= :start_date')
            filters.append('"dataid" < :end_date')
            params["start_date"] = _start_date
            params["end_date"] = _end_date

        where_clause = ""
        if filters:
            where_clause = " WHERE " + " AND ".join(filters)

        query_data = f"""
            SELECT * FROM hydroponic_data
            {where_clause}
            ORDER BY "dataid" DESC
            LIMIT :limit OFFSET :offset
        """

        query_count = f"""
            SELECT COUNT(*) FROM hydroponic_data
            {where_clause}
        """

        data_result = await self.session.execute(
            text(query_data),
            params,
        )
        count_result = await self.session.execute(
            text(query_count),
            {k: v for k, v in params.items() if k in ["start_date", "end_date"]},
        )

        return ResponseList(
            meta=MetaData(
                total_rows=count_result.scalar_one_or_none(),
                limit=limit,
                offset=(page - 1) * limit,
            ),
            data=list(
                HydroponicOut.model_validate(record)
                for record in data_result.mappings()
            ),
        )

    async def get_specific_data(
        self,
        parameter: str,
        page: int = 1,
        limit: int = 25,
        start_date: str | None = None,
        end_date: str | None = None,
    ) -> ResponseList[HydroponicOut]:
        if parameter in GROUPS:
            _fields = GROUPS[parameter]

        elif parameter in SENSOR_FIELDS | ENVIRONMENT_FIELDS | ACTUATOR_FIELDS:
            _fields = {parameter}

        else:
            raise ValueError(f"Invalid parameter: {parameter}")

        columns = ", ".join(f'"{field}"' for field in _fields)

        filters = []
        params: dict[str, Any] = {"limit": limit, "offset": (page - 1) * limit}

        # Process date conversion to UUID v7
        _start_date = get_uuidv7_from_timestamp(start_date) if start_date else None
        _end_date = (
            get_uuidv7_from_timestamp(end_date, is_end=True) if end_date else None
        )

        if _start_date and _end_date:
            filters.append('"dataid" >= :start_date')
            filters.append('"dataid" < :end_date')
            params["start_date"] = _start_date
            params["end_date"] = _end_date

        where_clause = ""
        if filters:
            where_clause = " WHERE " + " AND ".join(filters)

        query_data = f"""
            SELECT "dataid", {columns} FROM hydroponic_data
            {where_clause}
            ORDER BY "dataid" DESC
            LIMIT :limit OFFSET :offset
        """

        query_count = f"""
            SELECT COUNT(*) FROM hydroponic_data
            {where_clause}
        """

        data_result = await self.session.execute(
            text(query_data),
            params,
        )
        count_result = await self.session.execute(
            text(query_count),
            {k: v for k, v in params.items() if k in ["start_date", "end_date"]},
        )

        return ResponseList(
            meta=MetaData(
                total_rows=count_result.scalar_one_or_none(),
                limit=limit,
                offset=(page - 1) * limit,
            ),
            data=list(
                HydroponicOut.model_validate(record)
                for record in data_result.mappings()
            ),
        )

    async def get_latest_data(self) -> HydroponicOut | None:
        stmt = text(
            """
            SELECT * FROM hydroponic_data ORDER BY "dataid" DESC LIMIT 1
            """
        )
        record = await self.session.execute(stmt)
        record = record.mappings().first()
        if record:
            return HydroponicOut.model_validate(record)
        return None
