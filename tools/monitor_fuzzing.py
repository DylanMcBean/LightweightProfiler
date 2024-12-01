import argparse
import logging
import os
import signal
import subprocess
import sched
import time
from pathlib import Path
import requests
from dotenv import load_dotenv

logging.basicConfig(level=logging.INFO, format='%(message)s')

load_dotenv()

TELEGRAM_BOT_TOKEN = os.getenv("TELEGRAM_BOT_TOKEN")
TELEGRAM_CHAT_ID = os.getenv("TELEGRAM_CHAT_ID")

def send_telegram_message(message, token, chat_id):
    url = f"https://api.telegram.org/bot{token}/sendMessage"
    payload = {
        'chat_id': chat_id,
        'text': message
    }
    response = requests.post(url, data=payload)
    return response.json()

class FuzzerMonitor:
    def __init__(self, output_dir, fuzzer_stats, max_time_without_finds, check_interval, telegram_bot_token, telegram_chat_id):
        self.output_dir = Path(output_dir)
        self.fuzzer_stats_file = fuzzer_stats
        self.max_time_without_finds = max_time_without_finds
        self.scheduler = sched.scheduler(time.time, time.sleep)
        self.check_interval = check_interval
        self.last_fuzzer_data = {}
        self.start_time = None
        self.telegram_token = telegram_bot_token
        self.chat_id = telegram_chat_id

    def read_fuzzer_file(self):
        """Efficiently reads the fuzzer stats file."""
        fuzzer_stats_path = self.output_dir / self.fuzzer_stats_file
        fuzzer_stats = {}
        try:
            with fuzzer_stats_path.open("r") as file:
                for line in file:
                    if ":" in line:
                        key, value = line.strip().split(":", 1)
                        fuzzer_stats[key.strip()] = int(value) if value.strip().isdigit() else value.strip()
        except FileNotFoundError:
            logging.error(f"{fuzzer_stats_path} does not exist.")
            return None
        except Exception as e:
            logging.error(f"An error occurred while reading {fuzzer_stats_path}: {e}")
            return None
        return fuzzer_stats

    @staticmethod
    def get_afl_fuzz_pids():
        """Get a list of PIDs for afl-fuzz processes."""
        try:
            pids = subprocess.check_output(["pgrep", "-f", "afl-fuzz"]).decode().split()
            return [int(pid) for pid in pids]
        except subprocess.CalledProcessError:
            return []

    @staticmethod
    def stop_fuzzer():
        """Stop afl-fuzz processes gracefully."""
        pids = FuzzerMonitor.get_afl_fuzz_pids()
        for pid in pids:
            logging.info(f"Stopping afl-fuzz process with PID {pid}")
            os.kill(pid, signal.SIGTERM)

    def check_fuzzer_status(self):
        """Check fuzzer status and schedule next check."""
        fuzzer_data = self.read_fuzzer_file()

        if fuzzer_data is not None:
            time_wo_finds = int(fuzzer_data.get("time_wo_finds", 0))
            
            curr_time = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime())

            cycles = f"{int(fuzzer_data.get('cycles_done', 0)):,}" if 'cycles_done' in fuzzer_data else 'N/A'
            corpus = f"{int(fuzzer_data.get('corpus_count', 0)):,}" if 'corpus_count' in fuzzer_data else 'N/A'
            crashes = f"{int(fuzzer_data.get('saved_crashes', 0)):,}" if 'saved_crashes' in fuzzer_data else 'N/A'
            hangs = f"{int(fuzzer_data.get('saved_hangs', 0)):,}" if 'saved_hangs' in fuzzer_data else 'N/A'
            execs = f"{int(fuzzer_data.get('execs_done', 0)):,}" if 'execs_done' in fuzzer_data else 'N/A'
            edges = f"{int(fuzzer_data.get('edges_found', 0)):,}" if 'edges_found' in fuzzer_data else 'N/A'
            total_edges = f"{int(fuzzer_data.get('total_edges', 0)):,}" if 'total_edges' in fuzzer_data else 'N/A'

            if time_wo_finds > self.max_time_without_finds:
                logging.info(f"Time since last new path: {time_wo_finds} seconds. No new paths found. Exiting.")
                self.stop_fuzzer()
                update_message = (
                    f"Final Update:\n"
                    f"Cycles: {cycles}, Corpus: {corpus}\n"
                    f"Crashes: {crashes}, Hangs: {hangs}\n"
                    f"Execs: {execs}, Edges: {edges}, Total: {total_edges}\n"
                )
                send_telegram_message(update_message, self.telegram_token, self.chat_id)
                return

            if self.last_fuzzer_data and (fuzzer_data.get("last_find", 0) != self.last_fuzzer_data.get("last_find", 0) or fuzzer_data.get("corpus_count", 0) != self.last_fuzzer_data.get("corpus_count", 0) or fuzzer_data.get("saved_crashes", 0) != self.last_fuzzer_data.get("saved_crashes", 0) or fuzzer_data.get("edges_found", 0) != self.last_fuzzer_data.get("edges_found", 0) or fuzzer_data.get("total_edges", 0) != self.last_fuzzer_data.get("total_edges", 0)):
                logging.info(f"| {curr_time:<20} | {cycles:<13} | {corpus:<13} | {crashes:<13} | {hangs:<11} | {execs:<13} | {edges:<11} | {total_edges:<13} |")
            elif self.last_fuzzer_data.get("last_find") is None:
                logging.info(f"| {'Current Time':<20} | {'Cycles Count':<13} | {'Corpus Count':<13} | {'Saved Crashes':<13} | {'Saved Hangs':<11} | {'Execs Done':<13} | {'Edges Found':<11} | {'Total Edges':<13} |")
                logging.info(f"| {curr_time:<20} | {cycles:<13} | {corpus:<13} | {crashes:<13} | {hangs:<11} | {execs:<13} | {edges:<11} | {total_edges:<13} |")

            if self.last_fuzzer_data and (fuzzer_data.get('saved_crashes', 0) != self.last_fuzzer_data.get('saved_crashes', 0)):
                update_message = (
                    f"Update:\n"
                    f"Cycles: {cycles}, Corpus: {corpus}\n"
                    f"Crashes: {crashes}, Hangs: {hangs}\n"
                    f"Execs: {execs}, Edges: {edges}, Total: {total_edges}\n"
                )
                send_telegram_message(update_message, self.telegram_token, self.chat_id)
        else:
            logging.debug("Fuzzer stats file not found or unreadable.")
            send_telegram_message("Could not find stats file, stopping fuzzing", self.telegram_token, self.chat_id)
            exit()

        self.last_fuzzer_data = fuzzer_data
        self.scheduler.enter(self.check_interval, 1, self.check_fuzzer_status)

    def run(self):
        send_telegram_message("Fuzzing started", self.telegram_token, self.chat_id)
        self.start_time = time.localtime()
        current_time = time.strftime('%Y-%m-%d %H:%M:%S', self.start_time)
        logging.info(f"Current Time: {current_time}, Output Dir: {self.output_dir}, Fuzzer Stats: {self.fuzzer_stats_file}, Max Time Without Finds: {self.max_time_without_finds}, Check Interval: {self.check_interval}")
        
        self.scheduler.enter(10, 1, self.check_fuzzer_status)
        self.scheduler.run()

if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Monitor and manage AFLplusplus fuzzing process based on activity.")
    parser.add_argument("--output-dir", default="/path/to/fuzzer/output", help="Directory for fuzzer output")
    parser.add_argument("--fuzzer-stats", default="fuzzer_stats", help="Fuzzer stats filename")
    parser.add_argument("--max-time-without-finds", type=int, default=3600, help="Max time in seconds without new paths before stopping the fuzzer")
    parser.add_argument("--check-interval", type=int, default=60, help="Interval in seconds between checks on fuzzer status")
    args = parser.parse_args()

    monitor = FuzzerMonitor(args.output_dir, args.fuzzer_stats, args.max_time_without_finds, args.check_interval, TELEGRAM_BOT_TOKEN, TELEGRAM_CHAT_ID)
    monitor.run()
