import sys
from serial_biqbin_maxcut import SerialBiqBinMaxCut


biqbin = SerialBiqBinMaxCut()

adj_matrix, num_vertices, num_edges, name = biqbin.read_maxcut_input(sys.argv[1])
params = biqbin.read_parameters_with_python(sys.argv[2])

# print(np.info(adj_matrix))

result = biqbin.compute(adj_matrix, num_vertices, num_edges, name, params)

