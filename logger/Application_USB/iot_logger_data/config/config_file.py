from config import ConfigInformation


class ConfigFile:
    def __init__(self):
        self.config_path = ConfigInformation.CONFIG_FILE

    def load_config(self):
        config = {}
        try:
            with open(self.config_path, "r") as file:
                for line in file:
                    if line.strip() and not line.startswith("#"):
                        key, value = line.strip().split("=", 1)
                        config[key.strip()] = value.strip()
        except FileNotFoundError:
            print(f"Config file {self.config_path} not found.")
        except Exception as e:
            print(f"Error loading config: {e}")
        return config

    def save_config(self, config):
        try:
            with open(self.config_path, "w") as file:
                for key, value in config.items():
                    file.write(f"{key}={value}\n")
        except Exception as e:
            print(f"Error saving config: {e}")
