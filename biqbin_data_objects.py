import ctypes
import numpy as np


class BiqBinParameters(ctypes.Structure):
        """ creates a struct to match emxArray_real_T """

        _fields_ = [
            ('init_bundle_iter', ctypes.c_int), 
            ('max_bundle_iter', ctypes.c_int), 
            ('min_outer_iter', ctypes.c_int), 
            ('max_outer_iter', ctypes.c_int), 
            ('violated_Ineq', ctypes.c_double),
            ('TriIneq', ctypes.c_int), 
            ('Pent_Trials', ctypes.c_int), 
            ('Hepta_Trials', ctypes.c_int), 
            ('include_Pent', ctypes.c_int), 
            ('include_Hepta', ctypes.c_int), 
            ('root', ctypes.c_int), 
            ('use_diff', ctypes.c_int), 
            ('time_limit', ctypes.c_int), 
            ('branchingStrategy', ctypes.c_int), 
            ('detailedOutput', ctypes.c_int), 
        ]


class MaxCutInputData(ctypes.Structure):
    _fields_ = [
        ('name', ctypes.c_char_p),
        ('num_vertices', ctypes.c_int),
        ('num_edges', ctypes.c_int),
        ('Adj', ctypes.POINTER(ctypes.c_double))
 #       ('Adj', np.ctypeslib.ndpointer(dtype=np.float64, ndim=2, flags='C_CONTIGUOUS')),
    ]