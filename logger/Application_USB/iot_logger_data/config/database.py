import os
import sys
import hashlib
import sqlite3
from .config import ConfigInformation


class DatabaseConnection:
    def __init__(self, query="", data=""):
        self.query = query
        self.data = data
        self._db_path = self._resource_path()

    @staticmethod
    def _resource_path():
        try:
            base_path = sys._MEIPASS
        except AttributeError:
            base_path = os.path.abspath(".")
        return os.path.join(base_path, ConfigInformation.DATABASE_FILE)

    def _execute(self, query, data, *, fetch=None, commit=False):
        conn = sqlite3.connect(self._db_path)
        try:
            cur = conn.cursor()
            cur.execute(query, data) if data else cur.execute(query)
            if commit:
                conn.commit()
                return None
            if fetch == "one":
                row = cur.fetchone()
                return row[0] if row else None
            return cur.fetchall()
        finally:
            conn.close()

    def get_data(self):
        return self._execute(self.query, self.data)

    def get_one_data(self):
        return self._execute(self.query, self.data, fetch="one")

    def execute(self):
        self._execute(self.query, self.data, commit=True)

    add_data = execute
    update_data = execute
    delete_data = execute

    @classmethod
    def has_users(cls):
        db = cls("SELECT COUNT(*) FROM users")
        count = db.get_one_data()
        return count is not None and int(count) > 0

    @classmethod
    def create_user(cls, username: str, password: str):
        password_hash = hashlib.sha256(password.encode("utf-8")).hexdigest()
        db = cls(
            "INSERT INTO users (username, password) VALUES (?, ?)",
            (username, password_hash),
        )
        db.add_data()

    @classmethod
    def verify_user(cls, username: str, password: str) -> bool:
        password_hash = hashlib.sha256(password.encode("utf-8")).hexdigest()
        db = cls(
            "SELECT COUNT(*) FROM users WHERE username = ? AND password = ?",
            (username, password_hash),
        )
        count = db.get_one_data()
        return count is not None and int(count) > 0
