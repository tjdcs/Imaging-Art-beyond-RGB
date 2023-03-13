""" processing_pipeline.py
Processing pipeline functions

Functions:
    processing_pipeline(packet)

Authors:
    Brendan Grau <https://github.com/Victoriam7>
    Keenan Miller <https://github.com/keenanm500>

License:
    © 2022 BeyondRGB
    This code is licensed under the MIT license (see LICENSE.txt for details)
"""
# Python Imports
import gc
import os.path

# Local Imports
from rgbio import save_image
from packet import extract_camsigs, RENDERABLES_START
from preprocessing import preprocess
from calibration import color_calibrate
from rendering import render
from spectral_reflectance import spectrally_transform
from verification import verify_color_transformation, verify_spectral_transformation

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
    camsigs = extract_camsigs(packet)
    color_calibrate(packet, camsigs)
    spectrally_transform(packet, camsigs)

    # Values for btrgb
    xyz, lab, ciede = verify_color_transformation(packet, camsigs)
    r_cam, RMSE = verify_spectral_transformation(packet, camsigs)

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
        packet.btrgb.create_output_file(basename + ".tiff", "CM")
        del res
        gc.collect()
        # increment pointer
        packet.subjptr = (packet.subjptr[0] + 2, packet.subjptr[1] + 2)

    return
