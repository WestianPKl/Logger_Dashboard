import os
import sys
import sqlite3
from config import ConfigInformation


class DatabaseConnection:
    def __init__(self, query, data=""):
        self.__path = ConfigInformation.DATABASE_FILE
        self.query = query
        self.data = data
        self.__relative_path = self.__resource_path()

    def __resource_path(self):
        try:
            base_path = sys._MEIPASS
        except Exception:
            base_path = os.path.abspath(".")
        return os.path.join(base_path, self.__path)

    def __init_connection(self):
        self.__conn = sqlite3.connect(self.__relative_path)
        self.__cur = self.__conn.cursor()

    def get_data(self):
        self.__init_connection()
        if self.data != "":
            self.__cur.execute(self.query, self.data)
        else:
            self.__cur.execute(self.query)
        data = self.__cur.fetchall()
        self.__conn.close()
        return data

    def get_one_data(self):
        self.__init_connection()
        if self.data != "":
            self.__cur.execute(self.query, self.data)
        else:
            self.__cur.execute(self.query)
        row = self.__cur.fetchone()
        data = row[0] if row else None
        self.__conn.close()
        return data

    def add_data(self):
        self.__init_connection()
        self.__cur.execute(self.query, self.data)
        self.__conn.commit()
        self.__conn.close()

    def update_data(self):
        self.__init_connection()
        self.__cur.execute(self.query, self.data)
        self.__conn.commit()
        self.__conn.close()

    def delete_data(self):
        self.__init_connection()
        self.__cur.execute(self.query, self.data)
        self.__conn.commit()
        self.__conn.close()
