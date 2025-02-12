import ctypes
import os
import logging

class SerialBiqBinMaxCut:
    def __init__(self, biqbin_path="biqbin.so"):
        # Set up logging
        logging.basicConfig(level=logging.INFO)
        self.logger = logging.getLogger(__name__)

        # Load the shared library
        if not os.path.exists(biqbin_path):
            raise FileNotFoundError(f"Shared library not found: {biqbin_path}")
        self.biqbin = ctypes.CDLL(os.path.abspath(biqbin_path))

        # Define the argument types and return type for main()
        self.biqbin.main.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_char_p)]
        self.biqbin.main.restype = ctypes.c_int

        # Initialize arguments
        self.argc = 3
        self.argv = (ctypes.c_char_p * (self.argc + 1))()

    def set_instances_path(self, filepath):
        if not os.path.exists(filepath):
            raise FileNotFoundError(f"Instance file not found: {filepath}")
        self.argv[1] = filepath.encode("utf-8")

    def set_params_path(self, filepath):
        if not os.path.exists(filepath):
            raise FileNotFoundError(f"Params file not found: {filepath}")
        self.argv[2] = filepath.encode("utf-8")

    def call_main(self):
        self.argv[0] = b"./biqbin"
        # Validate paths
        if not os.path.exists(self.argv[1]):
            raise FileNotFoundError(f"Instance file not found: {self.argv[1].decode('utf-8')}")
        if not os.path.exists(self.argv[2]):
            raise FileNotFoundError(f"Params file not found: {self.argv[2].decode('utf-8')}")
        self.argv[3] = None
        result = self.biqbin.main(self.argc, self.argv)
        return result

# # Example usage
if __name__ == "__main__":
    try:
        bqb = SerialBiqBinMaxCut()
        bqb.set_instances_path("test/Instances/rudy/g05_60.0")
        bqb.set_params_path("test/params")
        bqb.call_main()
    except Exception as e:
        print(f"Error: {e}")