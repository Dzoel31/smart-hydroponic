from pydantic_settings import BaseSettings, SettingsConfigDict
from pydantic import Field, computed_field


class Settings(BaseSettings):
    model_config = SettingsConfigDict(
        env_file=".env",
        env_file_encoding="utf-8",
        extra="ignore",
        case_sensitive=True,
    )

    DATABASE_HOST: str = Field(validation_alias="PGHOST")
    DATABASE_PORT: int = Field(validation_alias="PGPORT")
    DATABASE_USER: str = Field(validation_alias="PGUSER")
    DATABASE_PASSWORD: str = Field(validation_alias="PGPASSWORD")
    DATABASE_NAME: str = Field(validation_alias="PGDATABASE")
    ALGORITHM: str = "HS256"
    ACCESS_TOKEN_EXPIRE: str = Field(validation_alias="JWT_EXPIRES_IN")
    SECRET_KEY: str = Field(validation_alias="JWT_SECRET")
    SUPERUSER_USERNAME: str = Field(validation_alias="SUPERUSER_USERNAME")
    SUPERUSER_EMAIL: str = Field(validation_alias="SUPERUSER_EMAIL")
    SUPERUSER_PASSWORD: str = Field(validation_alias="SUPERUSER_PASSWORD")
    SUPERUSER_ROLE: str = Field(validation_alias="SUPERUSER_ROLE")

    @computed_field
    @property
    def DATABASE_URL(self) -> str:
        return f"postgresql+asyncpg://{self.DATABASE_USER}:{self.DATABASE_PASSWORD}@{self.DATABASE_HOST}:{self.DATABASE_PORT}/{self.DATABASE_NAME}"


settings = Settings()
