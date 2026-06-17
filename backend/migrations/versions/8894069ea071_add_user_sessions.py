"""add user sessions

Revision ID: 8894069ea071
Revises: c6b1a3f4e2d7
Create Date: 2026-06-17 11:21:53.319978

"""
from typing import Sequence, Union

from alembic import op
import sqlalchemy as sa


# revision identifiers, used by Alembic.
revision: str = '8894069ea071'
down_revision: Union[str, Sequence[str], None] = 'c6b1a3f4e2d7'
branch_labels: Union[str, Sequence[str], None] = None
depends_on: Union[str, Sequence[str], None] = None


def upgrade() -> None:
    op.create_table(
        "user_sessions",
        sa.Column("session_id", sa.String(length=255), nullable=False),
        sa.Column("userid", sa.UUID(), nullable=False),
        sa.Column("username", sa.String(length=50), nullable=False,
                  server_default=""),
        sa.Column("role", sa.String(length=20), nullable=False,
                  server_default="admin"),
        sa.Column("created_at", sa.DateTime(timezone=True),
                  nullable=False, server_default=sa.text("now()")),
        sa.Column("expires_at", sa.DateTime(timezone=True), nullable=False),
        sa.Column("is_active", sa.Boolean(), nullable=False,
                  server_default=sa.text("true")),

        sa.PrimaryKeyConstraint("session_id", name=op.f("pk_user_sessions")),
        sa.ForeignKeyConstraint(
            ["userid"],
            ["user_data.userid"],
            name=op.f("fk_user_sessions_userid_user_data"),
            ondelete="CASCADE",
        ),
    )
    op.create_index(
        "idx_user_sessions_userid_active",
        "user_sessions",
        ["userid"],
        postgresql_where=sa.text("is_active = TRUE"),
    )


def downgrade() -> None:
    op.drop_index("idx_user_sessions_userid_active",
                  table_name="user_sessions")
    op.drop_table("user_sessions")