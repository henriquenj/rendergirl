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

from ctypes import *
import os
import sys

class RenderGirl:
    """Class that loads the RenderGirl shared library and holds most of
    the wrappers. It is called from RenderGirlBlender class. The
    difference between RenderGirl and RenderGirlBlender is that
    RenderGirl class is a singleton that is not suppose to be deleted
    across several renderings. So it's useful to retain states and perform
    init operations.
    """

    def __init__(self):
        # the current instance of RenderGirlBlender running
        self.session = None
        self.device_selected = False


    def start(self):
        """ Start function load the shared library with C code
        @return -1 on error
        """
        # load shared library containing RenderGirl
        # it's installed alongide the .py files on the same folder
        render_girl_directory = os.path.dirname(os.path.realpath(__file__)) + os.sep
        render_girl_path = render_girl_directory + "RenderGirlBlender_x86"
        # rendergirl shared library name may change based on compilation details
        # such as target architecture, OS, etc
        is_64bits = sys.maxsize > 2**32
        if is_64bits:
            render_girl_path += "_64"
        # holds the shared library instance containing C function calls
        self.render_girl_shared = cdll.LoadLibrary(render_girl_path)

        # create pointer to function according to the expected type
        # from StartLogSystem
        C_LOG_FUNCTION = CFUNCTYPE(None, c_char_p, c_bool)
        self.c_log_function = C_LOG_FUNCTION(self.log_callback)
        self.render_girl_shared.StartLogSystem(self.c_log_function)
        # now start the raytracer
        error = self.render_girl_shared.StartRendergirl()
        if error != 0:
            self.render_girl_shared.FinishLogSystem()
            return -1

        self.device_selected = True
        print("RenderGirl started")
        return 0


    def log_callback(self, message, error):
        """Called from RenderGirlCore when there's a new message to be printed
        @param message Message of type c_char_p to be printed
        @param error Boolean telling if it's a error message
        """
        # Blender does not offer any reporting apart from the usual
        # print calls when there's no rendering going on, so we only
        # print messages through "report" function calls when we are
        # rendering
        log_message = ''
        if error:
            message_type = {'ERROR'}
            log_message = "*ERROR* " + message.decode("ascii")
        else:
            message_type = {'INFO'}
            log_message = message.decode("ascii")

        if self.session == None:
            # no rendering happening, use print
            print(log_message)
        else: # use "report"
            self.session.report(message_type,log_message);

    def finish(self):
        if self.device_selected == True:
            self.render_girl_shared.FinishRenderGirl()
        self.render_girl_shared.FinishLogSystem()
        print("Rendergirl finished")
