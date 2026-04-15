from pydantic import BaseModel
from fastapi import status


class MessageResponse(BaseModel):
    detail: str


responses_400 = {
    status.HTTP_400_BAD_REQUEST: {
        "model": MessageResponse,
        "description": "Bad Request",
    },
}

responses_401 = {
    status.HTTP_401_UNAUTHORIZED: {
        "model": MessageResponse,
        "description": "Unauthorized",
    },
}
responses_403 = {
    status.HTTP_403_FORBIDDEN: {"model": MessageResponse, "description": "Forbidden"},
}
responses_404 = {
    status.HTTP_404_NOT_FOUND: {"model": MessageResponse, "description": "Not Found"},
}
responses_500 = {
    status.HTTP_500_INTERNAL_SERVER_ERROR: {
        "model": MessageResponse,
        "description": "Internal Server Error",
    },
}
