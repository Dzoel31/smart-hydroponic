#!/bin/sh
set -e

echo "Waiting for database..."
until python - <<EOF
import asyncpg, asyncio, os
dsn = f"postgresql://{os.getenv('PGUSER')}:{os.getenv('PGPASSWORD')}@{os.getenv('PGHOST')}:{os.getenv('PGPORT')}/{os.getenv('PGDATABASE')}"

async def main():
    try:
        await asyncpg.connect(dsn)
    except Exception as e:
        print(f"Still waiting... ({e})")
        exit(1)
asyncio.run(main())
EOF
do
  sleep 2
done

echo "Running migrations..."
python -m alembic upgrade head
echo "Create superuser if not exists..."
python -m utils.superuser

exec "$@"
