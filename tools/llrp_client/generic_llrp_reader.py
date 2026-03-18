import socket
import struct
import threading
import time
import os
import config

class MockReader:
    def __init__(self):
        self.inventory_running = False

    def connect(self):
        print("MockReader connected")

    def disconnect(self):
        print("MockReader disconnected")

    def start_inventory(self):
        print("MockReader start inventory")
        self.inventory_running = True

    def stop_inventory(self):
        print("MockReader stop inventory")
        self.inventory_running = False

    def read_tags(self):
        return ["E2000017221101441890B3AA"]

class GenericLLRPReader:
    def __init__(self):
        # Use central config as the single source of truth for host/port
        self.host = getattr(config, 'DEFAULT_HOST', '169.254.116.164')
        self.port = int(getattr(config, 'DEFAULT_PORT', 5084))
        self.sock = None
        self.inventory_running = False
        self._reader_thread = None

    def connect(self):
        self.sock = socket.create_connection((self.host, self.port), timeout=5)
        self.sock.settimeout(2)
        print(f"GenericLLRPReader connected to {self.host}:{self.port}")

    def disconnect(self):
        try:
            if self.sock:
                self.sock.close()
        except Exception:
            pass
        print("GenericLLRPReader disconnected")

    def start_inventory(self):
        # minimal sequence
        self.send_header(20, 1)  # ADD_ROSPEC
        time.sleep(0.1)
        self.send_header(24, 2)  # ENABLE_ROSPEC
        time.sleep(0.1)
        self.send_header(22, 3)  # START_ROSPEC
        self.inventory_running = True
        self._reader_thread = threading.Thread(target=self._reader_loop, daemon=True)
        self._reader_thread.start()

    def stop_inventory(self):
        self.send_header(23, 4)  # STOP_ROSPEC
        self.inventory_running = False

    def read_tags(self):
        return []

    def send_header(self, msg_type, message_id):
        ver = 1
        length = 10
        header_field = (ver << 13) | (msg_type & 0x7FF)
        pkt = struct.pack('>HII', header_field & 0xFFFF, length, message_id)
        try:
            self.sock.sendall(pkt)
            print(f"Sent msg type {msg_type} id {message_id}")
        except Exception as e:
            print(f"Error sending header: {e}")

    def _reader_loop(self):
        while self.inventory_running:
            try:
                data = self.sock.recv(8192)
                if data:
                    print('Recv:', ' '.join(f"{x:02X}" for x in data))
            except socket.timeout:
                continue
            except Exception as e:
                print('Reader loop error:', e)
                break
