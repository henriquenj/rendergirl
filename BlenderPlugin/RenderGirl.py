# RenderGirl - OpenCL raytracer renderer
# Copyright (c) 2015, Henrique Jung, All rights reserved.

# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 3.0 of the License, or any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
# Lesser General Public License for more details.

# You should have received a copy of the GNU Lesser General Public
# License along with this program.

import ctypes
import os
import sys

class RenderGirl:
    """Class that loads the RenderGirl shared library and holds most of
    the wrappers. It is called from RenderGirlBlender class. The
    difference between RenderGirl and RenderGirlBlender is that
    RenderGirl class is a singleton that is not suppose to be deleted
    across several renderings. So it's useful to retain states.
    """
    # holds the shared library instance containing C function calls
    render_girl_shared = None

    def __init__(self):
        """ Init function load the shared library with C code """

        # load shared library containing RenderGirl
        # it's installed alongide the .py files on the same folder
        render_girl_directory = os.path.dirname(os.path.realpath(__file__)) + os.sep
        render_girl_path = render_girl_directory + "RenderGirlBlender_x86"
        # rendergirl shared library name may change based on compilation details
        # such as target architecture, OS, etc
        is_64bits = sys.maxsize > 2**32
        if is_64bits:
            render_girl_path += "_64"
        RenderGirl.render_girl_shared = ctypes.cdll.LoadLibrary(render_girl_path)

        error = RenderGirl.render_girl_shared.StartRendergirl();
        print("RenderGirl started with code: {0}".format(error))

    def __del__(self):
        print("Finishing RenderGirl")
