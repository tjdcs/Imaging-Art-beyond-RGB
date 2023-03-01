""" processing_pipeline.py
Processing pipeline functions

Functions:
    processing_pipeline(packet)

Authors:
    Brendan Grau <https://github.com/Victoriam7>

License:
    © 2022 BeyondRGB
    This code is licensed under the MIT license (see LICENSE.txt for details)
"""
import gc
import numpy as np
import os.path

from rgbio import save_image
from packet import getimg, genpatchlist, RENDERABLES_START
from preprocessing import preprocess
from calibration import color_calibrate
from rendering import render
from constants import TARGET_RADIUS, IMGTYPE_TARGET


def processing_pipeline(packet):
    """ Color calibration pipeline
    [in] packet : packet to send through the pipeline
    """
    """ Calibration
    We need to get the calibration matrices for both color transformation and
    spectral imaging before we render all of the subjects including the target.
    See block comments for memory information
    """
    preprocess(packet)
    packet.camsigs = extract_camsigs(packet)
    color_calibrate(packet)

    """ Render and Save (Batch Processing)
    At this point we have the color transformation matrix and need to apply it
    to all images including the target. First we render the target in a special
    pass and then loop over all remaining subjects rendering and saving them
    one by one.
    """
    res = render(packet)
    basename = os.path.basename(packet.files[packet.subjptr[0]])
    basename = basename.split('.')[0]  # Trim extension
    save_image(res, packet.outpath, basename)
    del res
    gc.collect()

    # Set ptr to start of renderables
    packet.subjptr = (RENDERABLES_START, RENDERABLES_START + 1)
    while packet.subjptr[0] < len(packet.files):
        # generate file name
        basename = os.path.basename(packet.files[packet.subjptr[0]])
        basename = basename.split('.')[0]  # Trim extension
        # process, render, and save
        preprocess(packet)
        res = render(packet)
        save_image(res, packet.outpath, basename)
        del res
        gc.collect()
        # increment pointer
        packet.subjptr = (packet.subjptr[0] + 2, packet.subjptr[1] + 2)

    return


def extract_camsigs(packet):
    """ Generate camsigs array
    [in] packet : pipeline packet
    [out] camsigs array
    """
    t_img = getimg(packet, IMGTYPE_TARGET)
    siglist = genpatchlist(packet.target)
    tr = TARGET_RADIUS
    camsigs = np.ndarray((6, 130))
    for i, sig in enumerate(siglist):
        cell = t_img[0][sig[1]-tr:sig[1]+tr, sig[0]-tr:sig[0]+tr]
        avg = np.average(cell, axis=(0, 1))
        camsigs[0, i] = avg[0]
        camsigs[1, i] = avg[1]
        camsigs[2, i] = avg[2]
        cell = t_img[1][sig[1]-tr:sig[1]+tr, sig[0]-tr:sig[0]+tr]
        avg = np.average(cell, axis=(0, 1))
        camsigs[3, i] = avg[0]
        camsigs[4, i] = avg[1]
        camsigs[5, i] = avg[2]

    return camsigs
