from config.config import settings
from pathlib import Path

try:
    PRIVATE_KEY = Path(settings.JWT_PRIVATE_KEY).read_text(encoding="utf-8")
    PUBLIC_KEY = Path(settings.JWT_PUBLIC_KEY).read_text(encoding="utf-8")
except FileNotFoundError as e:
    raise RuntimeError(f"JWT key file not found: {e.filename}") from e  


def load_signing_key():
    if settings.ALGORITHM != "ES256":
        raise ValueError(f"Unsupported algorithm: {settings.ALGORITHM}")
    return PRIVATE_KEY


def load_verification_key():
    if settings.ALGORITHM != "ES256":
        raise ValueError(f"Unsupported algorithm: {settings.ALGORITHM}")
    return PUBLIC_KEY