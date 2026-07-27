"""create_plant_nutrition_profiles

Revision ID: c6b1a3f4e2d7
Revises: 3c2d9f7b8a11
Create Date: 2026-05-16 12:00:00.000000

"""

from typing import Sequence, Union

from alembic import op
import sqlalchemy as sa


# revision identifiers, used by Alembic.
revision: str = "c6b1a3f4e2d7"
down_revision: Union[str, Sequence[str], None] = "3c2d9f7b8a11"
branch_labels: Union[str, Sequence[str], None] = None
depends_on: Union[str, Sequence[str], None] = None


def upgrade() -> None:
    """Upgrade schema."""
    op.create_table(
        "plant_nutrition_profiles",
        sa.Column("nutrition_id", sa.UUID(), nullable=False),
        sa.Column("plant_name", sa.String(length=120), nullable=False),
        sa.Column("is_active", sa.Boolean(), nullable=False, server_default=sa.text("false")),
        sa.Column("moisture_min", sa.DOUBLE_PRECISION(), nullable=False),
        sa.Column("moisture_max", sa.DOUBLE_PRECISION(), nullable=False),
        sa.Column("ph_min", sa.DOUBLE_PRECISION(), nullable=False),
        sa.Column("ph_max", sa.DOUBLE_PRECISION(), nullable=False),
        sa.Column("tds_min", sa.DOUBLE_PRECISION(), nullable=False),
        sa.Column("tds_max", sa.DOUBLE_PRECISION(), nullable=False),
        sa.Column("temperature_min", sa.DOUBLE_PRECISION(), nullable=False),
        sa.Column("temperature_max", sa.DOUBLE_PRECISION(), nullable=False),
        sa.Column("humidity_min", sa.DOUBLE_PRECISION(), nullable=False),
        sa.Column("humidity_max", sa.DOUBLE_PRECISION(), nullable=False),
        sa.Column("notes", sa.Text(), nullable=True),
        sa.Column(
            "created_at",
            sa.DateTime(timezone=True),
            server_default=sa.text("CURRENT_TIMESTAMP"),
            nullable=False,
        ),
        sa.Column(
            "updated_at",
            sa.DateTime(timezone=True),
            server_default=sa.text("CURRENT_TIMESTAMP"),
            nullable=False,
        ),
        sa.PrimaryKeyConstraint("nutrition_id", name=op.f("pk_plant_nutrition_profiles")),
        sa.UniqueConstraint("plant_name", name=op.f("uq_plant_nutrition_profiles_plant_name")),
    )
    op.create_index(
        op.f("ix_plant_nutrition_profiles_plant_name"),
        "plant_nutrition_profiles",
        ["plant_name"],
        unique=False,
    )


def downgrade() -> None:
    """Downgrade schema."""
    op.drop_index(op.f("ix_plant_nutrition_profiles_plant_name"), table_name="plant_nutrition_profiles")
    op.drop_table("plant_nutrition_profiles")
