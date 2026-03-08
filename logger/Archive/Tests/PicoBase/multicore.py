import time, _thread


class MultiCoreTask:
    def __init__(self, target, args=()):
        self.target = target
        self.args = args
        self.thread_id = None

    def start(self):
        if self.thread_id is None:
            self.thread_id = _thread.start_new_thread(self.target, self.args)

    def is_running(self):
        return self.thread_id is not None

    def sleep(self, duration_ms):
        time.sleep_ms(duration_ms)

    def example_task(name, duration):
        for i in range(duration):
            print(f"Task {name} running: {i+1}/{duration}")
            time.sleep(1)


"""# Example usage:

def main():
    task1 = MultiCoreTask(target=MultiCoreTask.example_task, args=("A", 5))
    task2 = MultiCoreTask(target=MultiCoreTask.example_task, args=("B", 3))

    print("Starting tasks on multiple cores...")
    task1.start()
    task2.start()

    while task1.is_running() or task2.is_running():
        time.sleep(0.5)

    print("All tasks completed.")

if __name__ == "__main__":
    main()
    
"""
