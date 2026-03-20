import hashlib
from datetime import date

SECRET = "0xDG%#@!1137"


def generate_daily_password(day=None):
    if day is None:
        day = date.today()
    ymd = day.strftime("%Y%m%d")
    raw = f"{SECRET}:{ymd}"
    return hashlib.sha256(raw.encode("utf-8")).hexdigest().upper()[:10]


print(generate_daily_password())
