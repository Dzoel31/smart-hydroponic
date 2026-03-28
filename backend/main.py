from fastapi import FastAPI, Depends, Request
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import HTMLResponse
from fastapi.templating import Jinja2Templates
from sqlalchemy import text
from sqlalchemy.ext.asyncio import AsyncSession
from utils.deps import get_session
from routes.user_routes import router as user_router
from routes.hydroponic_routes import router as hydroponic_router

import logging

logging.basicConfig(
    level=logging.INFO,
    format="%(levelname)s:     %(message)s",
)
logger = logging.getLogger(__name__)


app = FastAPI(
    title="Smart Hydroponic API",
    version="2.0.0",
    root_path="/smart-hydroponic/api/v2",
    redoc_url=None,
    servers=[
        {
            "url": "http://localhost:8000/smart-hydroponic/api/v2",
            "description": "Local Development Server",
        },
    ],
)

templates = Jinja2Templates(directory="templates")

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_credentials=True,
    allow_methods=["*"],
    allow_headers=["*"],
)

app.include_router(user_router)
app.include_router(hydroponic_router)


@app.get("/", response_class=HTMLResponse)
async def root(request: Request):
    return templates.TemplateResponse("index.html", {"request": request})


@app.get("/health")
async def health():
    return {"status": "healthy"}


@app.get("/db-test")
async def db_test(session: AsyncSession = Depends(get_session)):
    result = await session.execute(text("SELECT 1"))
    return {"result": result.scalar()}
