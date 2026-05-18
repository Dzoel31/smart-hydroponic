from fastapi import APIRouter, Depends, HTTPException
from sqlalchemy.ext.asyncio import AsyncSession
from sqlalchemy.exc import IntegrityError, SQLAlchemyError
from uuid import UUID

from schemas.nutrition import (
    PlantNutritionProfileCreate,
    PlantNutritionProfileOut,
    PlantNutritionProfileUpdate,
)
from schemas.hydroponic import ResponseList
from schemas.responses import (
    responses_400,
    responses_401,
    responses_403,
    responses_404,
    responses_500,
)
from schemas.user import UserOut
from services.nutrition_service import NutritionService
from utils.deps import get_current_user, get_session, require_role

router = APIRouter(prefix="/nutrition", tags=["Nutrition"])


@router.get(
    "/profiles",
    response_model=ResponseList[PlantNutritionProfileOut],
    operation_id="getNutritionProfiles",
)
async def get_nutrition_profiles(
    page: int = 1,
    limit: int = 25,
    session: AsyncSession = Depends(get_session),
)-> ResponseList[PlantNutritionProfileOut]:
    service = NutritionService(session)
    return await service.get_profiles(page=page, limit=limit)


@router.get(
    "/profiles/active",
    response_model=PlantNutritionProfileOut,
    operation_id="getActiveNutritionProfile",
    responses={
        **responses_404,
        **responses_500,
    },
)
async def get_active_nutrition_profile(
    session: AsyncSession = Depends(get_session),
) -> PlantNutritionProfileOut:
    service = NutritionService(session)
    profile = await service.get_active_profile()
    if profile is None:
        raise HTTPException(status_code=404, detail="Nutrition profile not found")
    return PlantNutritionProfileOut.model_validate(profile)


@router.get(
    "/profiles/{nutrition_id}",
    response_model=PlantNutritionProfileOut,
    operation_id="getNutritionProfileById",
    responses={
        **responses_404,
        **responses_500,
    },
)
async def get_nutrition_profile_by_id(
    nutrition_id: UUID,
    session: AsyncSession = Depends(get_session),
) -> PlantNutritionProfileOut:
    service = NutritionService(session)
    profile = await service.get_profile_by_id(nutrition_id)
    if profile is None:
        raise HTTPException(status_code=404, detail="Nutrition profile not found")
    return PlantNutritionProfileOut.model_validate(profile)


@router.post(
    "/profiles",
    response_model=PlantNutritionProfileOut,
    status_code=201,
    operation_id="createNutritionProfile",
    responses={
        **responses_400,
        **responses_401,
        **responses_403,
        **responses_500,
    },
)
async def create_nutrition_profile(
    profile_data: PlantNutritionProfileCreate,
    current_user: UserOut = Depends(get_current_user),
    session: AsyncSession = Depends(get_session),
) -> PlantNutritionProfileOut:
    require_role(current_user, {"admin", "superadmin"})
    service = NutritionService(session)
    try:
        return await service.create_profile(profile_data)
    except IntegrityError:
        await session.rollback()
        raise HTTPException(status_code=400, detail="Plant name already registered")
    except SQLAlchemyError:
        await session.rollback()
        raise HTTPException(status_code=500, detail="Database error")


@router.patch(
    "/profiles/{nutrition_id}",
    response_model=PlantNutritionProfileOut,
    operation_id="updateNutritionProfile",
    responses={
        **responses_400,
        **responses_401,
        **responses_403,
        **responses_404,
        **responses_500,
    },
)
async def update_nutrition_profile(
    nutrition_id: UUID,
    profile_update: PlantNutritionProfileUpdate,
    current_user: UserOut = Depends(get_current_user),
    session: AsyncSession = Depends(get_session),
) -> PlantNutritionProfileOut:
    require_role(current_user, {"admin", "superadmin"})
    service = NutritionService(session)
    try:
        updated = await service.update_profile(nutrition_id, profile_update)
        if updated is None:
            raise HTTPException(status_code=404, detail="Nutrition profile not found")
        return updated
    except IntegrityError:
        await session.rollback()
        raise HTTPException(status_code=400, detail="Plant name already registered")
    except SQLAlchemyError:
        await session.rollback()
        raise HTTPException(status_code=500, detail="Database error")


@router.patch(
    "/profiles/{nutrition_id}/activate",
    response_model=PlantNutritionProfileOut,
    operation_id="activateNutritionProfile",
    responses={
        **responses_401,
        **responses_403,
        **responses_404,
        **responses_500,
    },
)
async def activate_nutrition_profile(
    nutrition_id: UUID,
    current_user: UserOut = Depends(get_current_user),
    session: AsyncSession = Depends(get_session),
) -> PlantNutritionProfileOut:
    require_role(current_user, {"admin", "superadmin"})
    service = NutritionService(session)
    try:
        activated = await service.set_active_profile(nutrition_id)
        if activated is None:
            raise HTTPException(status_code=404, detail="Nutrition profile not found")
        return activated
    except SQLAlchemyError:
        await session.rollback()
        raise HTTPException(status_code=500, detail="Database error")


@router.delete(
    "/profiles/{nutrition_id}",
    status_code=200,
    operation_id="deleteNutritionProfile",
    responses={
        **responses_401,
        **responses_403,
        **responses_404,
        **responses_500,
    },
)
async def delete_nutrition_profile(
    nutrition_id: UUID,
    current_user: UserOut = Depends(get_current_user),
    session: AsyncSession = Depends(get_session),
):
    require_role(current_user, {"admin", "superadmin"})
    service = NutritionService(session)
    try:
        deleted = await service.delete_profile(nutrition_id)
        if not deleted:
            raise HTTPException(status_code=404, detail="Nutrition profile not found")
        return {"detail": "Nutrition profile deleted successfully"}
    except SQLAlchemyError:
        await session.rollback()
        raise HTTPException(status_code=500, detail="Database error")
