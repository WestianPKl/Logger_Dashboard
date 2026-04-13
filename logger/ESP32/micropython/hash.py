import ubinascii
import uhashlib


class ServiceAuth:
    def __init__(self, secret_key):
        self.secret_key = secret_key

    def get_daily_password_for_date(self, ymd):
        raw = "{}:{}".format(self.secret_key, ymd)
        h = uhashlib.sha256(raw.encode("utf-8")).digest()
        return ubinascii.hexlify(h).decode("ascii").upper()[:10]

    def verify_password_for_date(self, password, ymd):
        if not password:
            return False
        return password.strip().upper() == self.get_daily_password_for_date(ymd)
