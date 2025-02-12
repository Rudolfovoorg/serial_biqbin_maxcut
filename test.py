import sys
from serial_biqbin_maxcut import SerialBiqBinMaxCut

path = 'test/Instances/rudy/'
biqbin = SerialBiqBinMaxCut()
biqbin.set_instances_path(path + 'g05_60.0')
biqbin.set_params_path(sys.argv[2])
biqbin.call_main()