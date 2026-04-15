from config.config import settings


def load_signing_key():
    if settings.ALGORITHM.startswith("HS"):
        return settings.SECRET_KEY.encode("utf-8")

    raise ValueError("Unsupported algorithm")


def load_verification_key():
    if settings.ALGORITHM.startswith("HS"):
        return settings.SECRET_KEY.encode("utf-8")

    raise ValueError("Unsupported algorithm")
