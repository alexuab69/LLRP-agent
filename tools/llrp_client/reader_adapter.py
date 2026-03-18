import os
import threading
from abc import ABC, abstractmethod
import config

class RFIDReader(ABC):
    @abstractmethod
    def connect(self):
        pass

    @abstractmethod
    def disconnect(self):
        pass

    @abstractmethod
    def start_inventory(self):
        pass

    @abstractmethod
    def stop_inventory(self):
        pass

    @abstractmethod
    def read_tags(self):
        pass

class ReaderAdapter:
    def __init__(self):
        requested = os.getenv('LLRP_READER_TYPE') or os.getenv('llrp.reader.type') or 'llrp'
        self.reader = None
        self.set_reader_type(requested)

    def set_reader_type(self, reader_type):
        # lazy import to avoid circular
        if self.reader:
            try:
                self.reader.disconnect()
            except Exception:
                pass

        if reader_type.lower() == 'mock':
            from .generic_llrp_reader import MockReader
            self.reader = MockReader()
        else:
            from .generic_llrp_reader import GenericLLRPReader
            self.reader = GenericLLRPReader()

        # For consistency, enforce central config host/port if reader supports it
        try:
            if hasattr(self.reader, 'host'):
                setattr(self.reader, 'host', getattr(config, 'DEFAULT_HOST', self.reader.host))
            if hasattr(self.reader, 'port'):
                setattr(self.reader, 'port', int(getattr(config, 'DEFAULT_PORT', self.reader.port)))
        except Exception:
            pass

        try:
            self.reader.connect()
        except Exception as e:
            print(f"Could not connect to reader ({reader_type}): {e}. Falling back to mock.")
            from .generic_llrp_reader import MockReader
            self.reader = MockReader()
            try:
                self.reader.connect()
            except Exception:
                pass

    def start_inventory(self):
        if self.reader:
            self.reader.start_inventory()

    def stop_inventory(self):
        if self.reader:
            self.reader.stop_inventory()

    def read_tags(self):
        if self.reader:
            return self.reader.read_tags()
        return []

    def is_inventory_running(self):
        if self.reader:
            return getattr(self.reader, 'inventory_running', False)
        return False
