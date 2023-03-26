""" rendering.py
Collection of functions for rendering an image

Functions:
    render(timgs)

Authors:
    Brendan Grau <https://github.com/Victoriam7>

License:
    © 2022 BeyondRGB
    This code is licensed under the MIT license (see LICENSE.txt for details)
"""
import numpy as np
import gc

from packet import getimg, IMGTYPE_SUBJECT


__PROPHOTO_TRANS = [[1.34594330, -0.2556075, -0.0511118],
                    [-0.5445989, 1.50816730, 0.02053510],
                    [0.00000000, 0.00000000, 1.21181280]]
__SRGB_TRANS = [[3.13385610, -1.6168667, -0.4906146],
                [-0.9787684, 1.91614150, 0.03345400],
                [0.07194530, -0.2289914, 1.40524270]]
__COLORSPACE = 'sRGB'  # 'sRGB' or 'ProPhoto'
__PROPHOTO_Y_LINE = 0.001953125
__SRGB_Y_LINE = 0.0031308


def render(packet):
    """ Render image pair as a single color calibrated image
    [in] packet : pipeline packet
    [out] rendered image
    [post] rendered image is loaded in memory
    [post] camsigs deleted from packet
    """
    camsigs, imgshape = __genimgsigs(packet)

    # Compute color calibrated image
    m = np.resize(packet.mo_matrix[0:18], (3, 6))
    o = np.resize(packet.mo_matrix[18:], (6, 1))
    xyz = np.matmul(m, np.subtract(camsigs, o))
    del camsigs
    gc.collect()

    # Convert to ProPhoto color space
    if __COLORSPACE == 'ProPhoto':
        rgb = np.matmul(__PROPHOTO_TRANS, xyz)
        del xyz
        gc.collect()
        np.clip(rgb, 0, 1, out=rgb)
        # Apply Gamma
        np.piecewise(rgb, [rgb > __PROPHOTO_Y_LINE, rgb <= __PROPHOTO_Y_LINE],
                     [lambda rgb: rgb ** (1/1.8), lambda rgb: rgb * 16])

    # Convert to sRGB color space
    if __COLORSPACE == 'sRGB':
        rgb = np.matmul(__PROPHOTO_TRANS, xyz)
        del xyz
        gc.collect()
        # Necessary to divide each value by 100, otherwise the image is completely white
        np.clip(rgb/100, 0, 1, out=rgb)
        # Apply Gamma
        np.piecewise(rgb, [rgb > __SRGB_Y_LINE, rgb <= __SRGB_Y_LINE],
                     [lambda rgb: (1.055 * (rgb ** (1/2.4))) - 0.055,
                      lambda rgb: 12.92 * rgb])

    # Reshape into image
    render = np.dstack((rgb[0], rgb[1], rgb[2])).reshape(imgshape)
    del rgb
    gc.collect()

    return np.float32(render)


def __genimgsigs(packet):
    """ Generate camsigs array for whole image
    [in] packet : pipeline packet
    [out] camsigs array
    [out] image dimentions
    """
    subj = getimg(packet, IMGTYPE_SUBJECT)

    shape = subj[0].shape
    camsigs = [subj[0][:, :, 0].flatten(),
               subj[0][:, :, 1].flatten(),
               subj[0][:, :, 2].flatten(),
               subj[1][:, :, 0].flatten(),
               subj[1][:, :, 1].flatten(),
               subj[1][:, :, 2].flatten()]

    del subj
    gc.collect()

    return camsigs, shape
