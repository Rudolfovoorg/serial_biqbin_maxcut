import sys
from serial_biqbin_maxcut import SerialBiqBinMaxCut

biqbin = SerialBiqBinMaxCut()

mxd, adjm = biqbin.read_maxcut_input(sys.argv[1])
params = biqbin.read_parameters(sys.argv[2])
result = biqbin.compute(mxd, params)