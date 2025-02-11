import sys
from serial_biqbin_maxcut import SerialBiqBinMaxCut


biqbin = SerialBiqBinMaxCut()
biqbin.set_instances_path(sys.argv[1])
biqbin.set_params_path(sys.argv[2])
biqbin.call_main()