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
        # first attempting to load realease version, and then debug versions
        try:
            RenderGirl.render_girl_shared = ctypes.cdll.LoadLibrary(
                "C:\\Users\\henrique\\Desktop\\Projetos\\rendergirl\\lib\\Release_x64\\RenderGirlBlender_x64")
        except Exception:
            pass
        # fall back to debug library
        if RenderGirl.render_girl_shared == None:
            RenderGirl.render_girl_shared = ctypes.cdll.LoadLibrary(
                "C:\\Users\\henrique\\Desktop\\Projetos\\rendergirl\\lib\\Debug_x64\\RenderGirlBlender_x64")
        # TODO: this will be hardcoded to my personal location for now
        # it's ungly, I know, it just for the beginning

        error = RenderGirl.render_girl_shared.StartRendergirl();
        print("RenderGirl started with code: {0}".format(error))

    def __del__(self):
        print("Finishing RenderGirl")
