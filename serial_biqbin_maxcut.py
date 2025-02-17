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

        # Read parameters
        self.biqbin.readParameters.argtypes = [ctypes.c_char_p]
        self.biqbin.readParameters.restype = BiqBinParameters

        self.biqbin.printParameters.argtypes = [BiqBinParameters]
        self.biqbin.printParameters.restype = None

        self.biqbin.printInputData.argtypes = [ctypes.POINTER(MaxCutInputData)]
        self.biqbin.printInputData.restype = None

    def compute(self, maxcut_data, params):
        return self.biqbin.compute(ctypes.pointer(maxcut_data), params)
    
    
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

    def read_parameters_with_biqbin(self, filepath):
        filepath_bytes = filepath.encode('utf-8')
        return self.biqbin.readParameters(filepath_bytes)
    
    def read_parameters_with_python(self, filename):
        params = BiqBinParameters()
        # Mapping field names to types
        field_types = {name: typ for name, typ in BiqBinParameters._fields_}
        with open(filename, 'r') as f:
            for line in f.readlines():
                key_val = line.strip().split('=')
                if len(key_val) != 2:
                    print(f"Skipping invalid line: {line.strip()}")
                    continue  # Skip invalid lines
                key, value = key_val[0].strip(), key_val[1].strip()
                if hasattr(params, key):
                    field_type = field_types[key]
                    # Convert value to the correct type
                    if field_type == ctypes.c_int:
                        setattr(params, key, int(value))
                    elif field_type == ctypes.c_double:
                        setattr(params, key, float(value))
                    else:
                        print(f"Unknown type for field: {key}")
                else:
                    print(f"Unknown parameter: {key}")
        return params  # Return the filled struct
    
    def print_parameters(self, params):
        self.biqbin.printParameters(params)

    def print_input_data(self, maxcut_data):
        self.biqbin.printInputData(ctypes.pointer(maxcut_data))