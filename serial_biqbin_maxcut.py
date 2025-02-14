import ctypes
import os
import numpy as np
from biqbin_data_objects import MaxCutInputData, BiqBinParameters

class SerialBiqBinMaxCut:
    def __init__(self, biqbin_path="biqbin.so"):
        # Load the shared library
        if not os.path.exists(biqbin_path):
            raise FileNotFoundError(f"Shared library not found: {biqbin_path}")
        self.biqbin = ctypes.CDLL(os.path.abspath(biqbin_path))

        self.biqbin.compute.argtypes = [ctypes.POINTER(MaxCutInputData), BiqBinParameters]
        self.biqbin.compute.restype = ctypes.c_int

        # Define the argument types and return type for main()
        self.biqbin.main.argtypes = [ctypes.c_int, ctypes.POINTER(ctypes.c_char_p)]
        self.biqbin.main.restype = ctypes.c_int

        # parse matrix
        self.biqbin.processAdjMatrixSetPP_SP.argtypes = [ctypes.POINTER(MaxCutInputData)]
        self.biqbin.processAdjMatrixSetPP_SP.restype = None

        # Read parameters
        self.biqbin.readParameters.argtypes = [ctypes.c_char_p]
        self.biqbin.readParameters.restype = BiqBinParameters

        self.biqbin.printParameters.argtypes = [BiqBinParameters]
        self.biqbin.printParameters.restype = None

        self.biqbin.printInputData.argtypes = [ctypes.POINTER(MaxCutInputData)]
        self.biqbin.printInputData.restype = None

        # Initialize arguments
        self.argc = 3
        self.argv = (ctypes.c_char_p * (self.argc + 1))()

    def compute(self, maxcut_data, params):
        return self.biqbin.compute(ctypes.pointer(maxcut_data), params)

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
    
    def process_adj_matrix_set_PP_SP(self, maxcut_data):
        self.biqbin.processAdjMatrixSetPP_SP(ctypes.pointer(maxcut_data))
    
    def read_maxcut_input(self, filename):
        with open(filename, 'r') as f:
            # Read number of vertices and edges
            num_vertices, num_edges = map(int, f.readline().split())

            # Allocate adjacency matrix as a contiguous array (row-major)
            adj_matrix = np.zeros((num_vertices, num_vertices), dtype=np.float64)

            # Read edges
            for _ in range(num_edges):
                i, j, weight = f.readline().split()
                i, j = int(i) - 1, int(j) - 1  # Convert to zero-based indexing
                weight = float(weight)

                adj_matrix[i, j] = weight
                adj_matrix[j, i] = weight  # Since the graph is undirected

            # Convert NumPy array to ctypes pointer (1D row-major)
            adj_ptr = adj_matrix.flatten().ctypes.data_as(ctypes.POINTER(ctypes.c_double))
            # Create the MaxCutInputData struct
            name = filename.encode('utf-8')
            maxcut_data = MaxCutInputData(name, num_vertices, num_edges, adj_ptr)
            maxcut_data._adj_matrix = adj_matrix  # Prevents premature deallocation

            return maxcut_data, adj_matrix  # Returning the matrix for debugging

    def read_parameters(self, filepath):
        filepath_bytes = filepath.encode('utf-8')
        return self.biqbin.readParameters(filepath_bytes)
    
    def print_parameters(self, params):
        self.biqbin.printParameters(params)

    def print_input_data(self, maxcut_data):
        self.biqbin.printInputData(ctypes.pointer(maxcut_data))

    def set_instances_path(self, filepath):
        if not os.path.exists(filepath):
            raise FileNotFoundError(f"Instance file not found: {filepath}")
        self.argv[1] = filepath.encode("utf-8")

    def set_params_path(self, filepath):
        if not os.path.exists(filepath):
            raise FileNotFoundError(f"Params file not found: {filepath}")
        self.argv[2] = filepath.encode("utf-8")
