from pydantic import BaseModel, Field, ConfigDict, model_validator
from pydantic.networks import EmailStr
from datetime import datetime
from typing import Optional
from uuid import UUID
from enum import Enum


class UserRole(str, Enum):
    ADMIN = "admin"
    USER = "user"
    SUPERADMIN = "superadmin"


class UserBase(BaseModel):
    username: str = Field(..., min_length=3, max_length=50)
    email: EmailStr


class UserCreate(UserBase):
    password: str = Field(..., min_length=8, max_length=255)
    role: UserRole


class UserLogin(BaseModel):
    username: Optional[str] = None
    email: Optional[EmailStr] = None
    password: str

    @model_validator(mode="after")
    def check_username_or_email(self):
        if not self.username and not self.email:
            raise ValueError("Either username or email must be provided.")
        return self


class UserUpdate(BaseModel):
    username: Optional[str] = Field(None, min_length=3, max_length=50)
    fullname: Optional[str] = Field(None, min_length=3, max_length=100)
    phone_number: Optional[str] = Field(None, min_length=10, max_length=20)
    password: Optional[str] = Field(None, min_length=8, max_length=255)
    role: Optional[UserRole] = None


class UserOut(UserBase):
    userid: UUID
    email: Optional[EmailStr] = None
    fullname: Optional[str] = None
    phone_number: Optional[str] = None
    role: UserRole
    created_at: datetime

    model_config = ConfigDict(from_attributes=True)


class PasswordChange(BaseModel):
    current_password: str = Field(..., min_length=8, max_length=255)
    new_password: str = Field(..., min_length=8, max_length=255)


class Token(BaseModel):
    access_token: str
    token_type: str = "Bearer"


class AccountSummary(BaseModel):
    userid: UUID
    username: str
    email: Optional[EmailStr] = None
    fullname: Optional[str] = None
    role: UserRole


class LoginResponse(Token):
    user: AccountSummary


class TokenPayload(BaseModel):
    sub: str
    id: str
    role: str
    tv: int = 0
    iat: int
    exp: int
