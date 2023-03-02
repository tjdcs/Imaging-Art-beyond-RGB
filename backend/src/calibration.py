""" calibration.py
Collection of functions for color calibration

Functions:
    color_calibrate(packet)

Authors:
    Brendan Grau <https://github.com/Victoriam7>

License:
    © 2022 BeyondRGB
    This code is licensed under the MIT license (see LICENSE.txt for details)
"""
# Python Imports
import gc
import numpy as np
from scipy.optimize import fmin

# Local Imports
from spectral_equation import xyztolab, ciede2000
from packet import Packet


__INIT_MOARR = [0.10, 0.10, 0.25, 0.50, 0.10, 0.10,
                0.10, 0.10, 0.25, 0.10, 1.00, 0.10,
                0.10, 0.10, 0.25, 0.10, 0.10, 0.50,
                0.01, 0.01, 0.01, 0.01, 0.01, 0.01]


def color_calibrate(packet: Packet, camsigs: np.ndarray):
    """ Process target images to get the calibrated M matrix
    [in] packet  : pipeline packet
    [in] camsigs : target camera signals
    [post] packet x variable is populated
    """
    ref = packet.target.lab_ref
    res = fmin(__de_equ, __INIT_MOARR, (camsigs, ref))
    print(__de_equ(res, camsigs, ref))
    packet.x = res


def __de_equ(x: np.ndarray, camsigs: np.ndarray, labref: np.ndarray) -> float:
    """ ∆E equation for color tranformation matrix optimization
    [in] x : current input parameters guess
    [in] camsigs : target camera signals we are modifying
    [in] labref  : target reference data
    [out] average error
    """
    m = np.resize(x[0:18], (3, 6))
    o = np.resize(x[18:], (6, 1))
    xyz = np.matmul(m, np.subtract(camsigs, o))
    lab = xyztolab(xyz)
    xyzshape = xyz.shape[1]
    del xyz
    gc.collect()
    ciede = np.zeros(xyzshape)
    for i in range(0, lab.shape[1]):
        ciede[i] = ciede2000(lab[:, i], labref[:, i])
    err = np.mean(ciede)
    del ciede
    gc.collect()
    return err
