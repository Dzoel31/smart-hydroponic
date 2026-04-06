from datetime import datetime, timezone
from pathlib import Path
import sys
from uuid import uuid4

import pytest
from fastapi import HTTPException
from fastapi.testclient import TestClient

sys.path.insert(0, str(Path(__file__).resolve().parents[1]))

from main import app
from schemas.user import UserOut
from services.hydroponic_service import HydroponicService
from services.user_service import UserService
from utils.deps import get_current_user, get_session


class _DummySession:
    async def execute(self, *_args, **_kwargs):
        return None


@pytest.fixture
def client():
    async def override_get_session():
        yield _DummySession()

    app.dependency_overrides[get_session] = override_get_session

    with TestClient(app, raise_server_exceptions=False) as test_client:
        yield test_client

    app.dependency_overrides.clear()


def _set_current_user_override(role: str = "admin"):
    async def override_get_current_user():
        return UserOut(
            userid=uuid4(),
            username="tester",
            email="tester@example.com",
            fullname="Test User",
            role=role,
            created_at=datetime.now(timezone.utc),
        )

    app.dependency_overrides[get_current_user] = override_get_current_user


def test_login_requires_username_or_email(client: TestClient):
    response = client.post(
        "/users/login",
        json={
            "password": "supersecret123",
        },
    )

    assert response.status_code == 422


def test_login_invalid_credentials(client: TestClient, monkeypatch: pytest.MonkeyPatch):
    async def fake_authenticate_user(_self, _credentials):
        raise HTTPException(status_code=401, detail="Invalid username or password")

    monkeypatch.setattr(UserService, "authenticate_user", fake_authenticate_user)

    response = client.post(
        "/users/login",
        json={
            "username": "newuser",
            "password": "wrong-password",
        },
    )

    assert response.status_code == 401
    assert response.json()["detail"] == "Invalid username or password"


def test_register_duplicate_username(
    client: TestClient, monkeypatch: pytest.MonkeyPatch
):
    async def fake_get_user_by_username(_self, _username):
        return {
            "userid": uuid4(),
            "username": "newuser",
            "email": "newuser@example.com",
            "password": "hashed",
            "role": "user",
            "created_at": datetime.now(timezone.utc),
            "is_superuser": False,
        }

    monkeypatch.setattr(UserService, "get_user_by_username", fake_get_user_by_username)

    response = client.post(
        "/users/register",
        json={
            "username": "newuser",
            "email": "newuser@example.com",
            "password": "supersecret123",
        },
    )

    assert response.status_code == 400
    assert response.json()["detail"] == "Username already registered"


def test_users_endpoint_requires_auth(client: TestClient):
    response = client.get("/users")

    assert response.status_code == 401
    assert response.json()["detail"] == "Not authenticated"


def test_delete_user_forbidden_for_user_role(client: TestClient):
    _set_current_user_override(role="user")

    response = client.delete(f"/users/{uuid4()}")

    assert response.status_code == 403
    assert response.json()["detail"] == "Permission denied"


def test_hydroponic_latest_no_data(client: TestClient, monkeypatch: pytest.MonkeyPatch):
    _set_current_user_override(role="admin")

    async def fake_get_latest_data(_self):
        return None

    monkeypatch.setattr(HydroponicService, "get_latest_data", fake_get_latest_data)

    response = client.get("/hydroponics/data/latest")

    assert response.status_code == 204


def test_hydroponic_specific_forbidden_for_user_role(client: TestClient):
    _set_current_user_override(role="user")

    response = client.get("/hydroponics/data/ph")

    assert response.status_code == 403
    assert response.json()["detail"] == "Permission denied"


def test_hydroponic_post_forbidden_for_user_role(client: TestClient):
    _set_current_user_override(role="user")

    response = client.post("/hydroponics/data", json={})

    assert response.status_code == 403
    assert response.json()["detail"] == "Permission denied"


def test_hydroponic_specific_invalid_parameter(
    client: TestClient, monkeypatch: pytest.MonkeyPatch
):
    _set_current_user_override(role="admin")

    async def fake_get_specific_data(
        _self,
        _parameter,
        _page=1,
        _limit=25,
        _start_date=None,
        _end_date=None,
    ):
        raise ValueError("Invalid parameter: invalid_field")

    monkeypatch.setattr(HydroponicService, "get_specific_data", fake_get_specific_data)

    response = client.get("/hydroponics/data/invalid_field")

    assert response.status_code == 400
    assert response.json()["detail"] == "Invalid parameter: invalid_field"
