import datetime
import unittest

from utils.converter import get_uuidv7_from_timestamp


def _extract_timestamp_ms(value):
    return (value.int >> 80) & 0xFFFFFFFFFFFF


class TestConverter(unittest.TestCase):
    def test_parses_unix_milliseconds_int(self):
        raw_ms = 1773705600000
        result = get_uuidv7_from_timestamp(raw_ms)

        self.assertEqual(_extract_timestamp_ms(result), raw_ms)

    def test_parses_unix_seconds_int(self):
        raw_seconds = 1773705600
        expected_ms = int(
            datetime.datetime.fromtimestamp(
                raw_seconds, tz=datetime.timezone.utc
            ).timestamp()
            * 1000
        )

        result = get_uuidv7_from_timestamp(raw_seconds)
        self.assertEqual(_extract_timestamp_ms(result), expected_ms)

    def test_parses_numeric_string_timestamp(self):
        raw_ms = 1773705600000
        result = get_uuidv7_from_timestamp(str(raw_ms))

        self.assertEqual(_extract_timestamp_ms(result), raw_ms)

    def test_parses_iso_z_datetime(self):
        value = "2026-03-15T12:30:00Z"
        expected_ms = int(
            datetime.datetime(
                2026, 3, 15, 12, 30, 0, tzinfo=datetime.timezone.utc
            ).timestamp()
            * 1000
        )

        result = get_uuidv7_from_timestamp(value)
        self.assertEqual(_extract_timestamp_ms(result), expected_ms)

    def test_parses_custom_dd_mm_yyyy_datetime(self):
        value = "15/03/2026 12:30:00"
        expected_ms = int(
            datetime.datetime(
                2026, 3, 15, 12, 30, 0, tzinfo=datetime.timezone.utc
            ).timestamp()
            * 1000
        )

        result = get_uuidv7_from_timestamp(value)
        self.assertEqual(_extract_timestamp_ms(result), expected_ms)

    def test_parses_rfc2822_datetime(self):
        value = "Sun, 15 Mar 2026 12:30:00 GMT"
        expected_ms = int(
            datetime.datetime(
                2026, 3, 15, 12, 30, 0, tzinfo=datetime.timezone.utc
            ).timestamp()
            * 1000
        )

        result = get_uuidv7_from_timestamp(value)
        self.assertEqual(_extract_timestamp_ms(result), expected_ms)

    def test_date_only_start_and_end_boundaries(self):
        start = get_uuidv7_from_timestamp("2026-03-15", is_end=False)
        end = get_uuidv7_from_timestamp("2026-03-15", is_end=True)

        expected_start_ms = int(
            datetime.datetime(
                2026, 3, 15, 0, 0, 0, tzinfo=datetime.timezone.utc
            ).timestamp()
            * 1000
        )
        expected_end_ms = int(
            datetime.datetime(
                2026, 3, 15, 23, 59, 59, 999999, tzinfo=datetime.timezone.utc
            ).timestamp()
            * 1000
        )

        self.assertEqual(_extract_timestamp_ms(start), expected_start_ms)
        self.assertEqual(_extract_timestamp_ms(end), expected_end_ms)

    def test_parses_wib_datetime(self):
        value = "2026-03-15T12:30:00+07:00"
        wib_tz = datetime.timezone(datetime.timedelta(hours=7))
        expected_ms = int(
            datetime.datetime(2026, 3, 15, 12, 30, 0, tzinfo=wib_tz).timestamp() * 1000
        )

        result = get_uuidv7_from_timestamp(value)
        self.assertEqual(_extract_timestamp_ms(result), expected_ms)

    def test_raises_for_invalid_format(self):
        with self.assertRaises(ValueError):
            get_uuidv7_from_timestamp("not-a-date")


if __name__ == "__main__":
    unittest.main()
