import datetime
from email.utils import parsedate_to_datetime
from uuid import UUID


def _parse_unix_timestamp(value: int | float) -> datetime.datetime:
    # Values above this threshold are treated as milliseconds, otherwise seconds.
    timestamp_seconds = value / 1000 if abs(value) >= 100_000_000_000 else value
    return datetime.datetime.fromtimestamp(timestamp_seconds, tz=datetime.timezone.utc)


def _parse_datetime_input(value: str | int) -> tuple[datetime.datetime, bool]:
    is_date_only = False

    if isinstance(value, int):
        return _parse_unix_timestamp(value), is_date_only

    normalized = value.strip()
    if not normalized:
        raise ValueError("Empty time value")

    if normalized.lstrip("+-").isdigit():
        return _parse_unix_timestamp(int(normalized)), is_date_only

    try:
        date_only = datetime.date.fromisoformat(normalized)
        return datetime.datetime.combine(date_only, datetime.time.min), True
    except ValueError:
        pass

    try:
        dt = datetime.datetime.fromisoformat(normalized.replace("Z", "+00:00"))
        return dt, is_date_only
    except ValueError:
        pass

    datetime_formats = [
        "%Y-%m-%d %H:%M:%S",
        "%Y/%m/%d %H:%M:%S",
        "%d/%m/%Y %H:%M:%S",
        "%m/%d/%Y %H:%M:%S",
    ]
    for fmt in datetime_formats:
        try:
            return datetime.datetime.strptime(normalized, fmt), is_date_only
        except ValueError:
            continue

    date_formats = [
        "%Y/%m/%d",
        "%d/%m/%Y",
        "%m/%d/%Y",
    ]
    for fmt in date_formats:
        try:
            date_only = datetime.datetime.strptime(normalized, fmt).date()
            return datetime.datetime.combine(date_only, datetime.time.min), True
        except ValueError:
            continue

    try:
        return parsedate_to_datetime(normalized), is_date_only
    except (TypeError, ValueError):
        pass

    raise ValueError(f"Invalid time format: {value}")


def get_uuidv7_from_timestamp(time_str: str | int, is_end=False) -> UUID:
    try:
        dt, is_date_only = _parse_datetime_input(time_str)
    except ValueError as e:
        raise ValueError(f"Invalid time format: {time_str}") from e
    except (OSError, OverflowError) as e:
        raise ValueError(f"Timestamp out of range: {time_str}") from e

    if is_date_only:
        target_time = datetime.time.max if is_end else datetime.time.min
        dt = datetime.datetime.combine(dt.date(), target_time)

    if dt.tzinfo is None:
        dt = dt.replace(tzinfo=datetime.timezone.utc)
    else:
        dt = dt.astimezone(datetime.timezone.utc)

    timestamp_ms = int(dt.timestamp() * 1000)

    uuid_int = (timestamp_ms & 0xFFFFFFFFFFFF) << 80

    uuid_int |= 0x7 << 76  # Set version to 7

    uuid_int |= 0x2 << 62  # Set variant to RFC 4122

    if is_end:
        uuid_int |= 0x000000000FFF3FFFFFFFFFFFFFFF

    return UUID(int=uuid_int)
