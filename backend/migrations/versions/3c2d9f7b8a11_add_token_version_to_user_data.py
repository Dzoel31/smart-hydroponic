"""Add token version to user_data

Revision ID: 3c2d9f7b8a11
Revises: 5f70a1ebe4eb
Create Date: 2026-04-08 10:00:00.000000

"""

from typing import Sequence, Union

from alembic import op
import sqlalchemy as sa


# revision identifiers, used by Alembic.
revision: str = "3c2d9f7b8a11"
down_revision: Union[str, Sequence[str], None] = "a11a457ed31c"
branch_labels: Union[str, Sequence[str], None] = None
depends_on: Union[str, Sequence[str], None] = None


def upgrade() -> None:
    """Upgrade schema."""
    op.add_column(
        "user_data",
        sa.Column("token_version", sa.Integer(), nullable=False, server_default="0"),
    )


def downgrade() -> None:
    """Downgrade schema."""
    op.drop_column("user_data", "token_version")
