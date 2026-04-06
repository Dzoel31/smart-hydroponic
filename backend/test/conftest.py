import os

# Provide default settings so imports that instantiate Settings() do not fail in tests.
os.environ.setdefault("PGHOST", "localhost")
os.environ.setdefault("PGPORT", "5432")
os.environ.setdefault("PGUSER", "test_user")
os.environ.setdefault("PGPASSWORD", "test_password")
os.environ.setdefault("PGDATABASE", "test_db")
os.environ.setdefault("JWT_EXPIRES_IN", "1h")
os.environ.setdefault("JWT_SECRET", "test-secret-key")
os.environ.setdefault("SUPERUSER_USERNAME", "superadmin")
os.environ.setdefault("SUPERUSER_EMAIL", "superadmin@example.com")
os.environ.setdefault("SUPERUSER_PASSWORD", "supersecret123")
os.environ.setdefault("SUPERUSER_ROLE", "superadmin")
