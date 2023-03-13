""" calibration.py
Collection of functions for color calibration

Functions:
    color_calibrate(packet)

Authors:
    Brendan Grau <https://github.com/Victoriam7>
    Keenan Miller <https://github.com/keenanm500>

License:
    © 2022 BeyondRGB
    This code is licensed under the MIT license (see LICENSE.txt for details)
"""
# Python Imports
import gc
import numpy as np
from scipy.optimize import fmin

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
    res = fmin(__de_equ, __INIT_MOARR, (camsigs, ref, packet.btrgb),
               maxfun=5000, maxiter=5000)
    print(__de_equ(res, camsigs, ref))
    packet.mo_matrix = res


def __de_equ(x: np.ndarray, camsigs: np.ndarray, labref: np.ndarray, btrgb) -> float:
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
    btrgb.CalibrationResults["matrix_values"][4] = btrgb.create_matrix_value(lab[0], "CM L*_camera")
    btrgb.CalibrationResults["matrix_values"][5] = btrgb.create_matrix_value(lab[1], "CM a*_camera")
    btrgb.CalibrationResults["matrix_values"][6] = btrgb.create_matrix_value(lab[2], "CM b*_camera")
    xyzshape = xyz.shape[1]
    ciede = np.zeros(xyzshape)
    for i in range(0, lab.shape[1]):
        ciede[i] = ciede2000(lab[:, i], labref[:, i])
    btrgb.CalibrationResults["matrix_values"][0] = btrgb.create_matrix_value(ciede, "CM DeltaE Values")
    err = np.mean(ciede)
    btrgb.CalibrationResults["double_values"][0] = btrgb.create_double_value(err, "CM DeltaE Mean")
    return err
