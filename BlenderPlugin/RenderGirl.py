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

import bmesh

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
        # set source path for .cl sources (the same directory as everything else)
        self.render_girl_shared.SetSourcePath(render_girl_directory.encode("ascii"))
        # now start the raytracer
        error = self.render_girl_shared.StartRendergirl()
        if error != 0:
            self.render_girl_shared.FinishLogSystem()
            return -1

        self.device_selected = True
        print("RenderGirl started")
        return 0

    def add_scene_group(self, bmesh_tuple):
        """Add an object to RenderGirl scene
        @param bmesh_tuple namedtuple composed of a tringulated bmesh
        alongside its name, position, scale and rotation
        @return 0 for success, -1 for error
        """
        if bmesh_tuple == None:
            return -1

        # blender 2.73 introduced "ensure_lookup_table" option for
        # faster geometry lookup, more info at
        # http://wiki.blender.org/index.php/Dev:Ref/Release_Notes/2.73/Addons
        # since we need faces -> vert mapping we will have to call at
        # least for faces
        if hasattr(bmesh_tuple.bmesh.faces, "ensure_lookup_table"):
            bmesh_tuple.bmesh.faces.ensure_lookup_table()

        faces_list = [] # each face is composed of three integer that
                        # points to the vertex dictionary
        vertex_dict = {} # vertex_dict will hold a pointer to vertex
                         # object (which will act as key) alongside an
                         # integer, this integer is used by faces_list
                         # indexing. Dictionary properties will make
                         # sure vertices are not repeated
        vertex_dict_last_index = 0 # next index to be added to
                                   # vertex_dict
        for i in range(len(bmesh_tuple.bmesh.faces)):
            for vert in bmesh_tuple.bmesh.faces[i].verts:
                if vert not in vertex_dict:
                    # vertex will be added for the first time
                    vertex_dict[vert] = vertex_dict_last_index
                    faces_list.append(vertex_dict_last_index)
                    vertex_dict_last_index += 1
                else:
                    # vertex will be retrived
                    faces_list.append(vertex_dict.get(vert))

        # Since elements not ordered in the dictionary, we
        # will have to use an aux list with the elements sorted by
        # value
        ordered_vertex = sorted(vertex_dict,key=vertex_dict.get)

        # the faces look like this
        # faces_list = [ 1, 2, 4 , 4, 5, 6 , 6, 4, 3 ]
        # each of them points to a position in ordered_vertex
        c_faces_buffer = (c_int * len(faces_list))(*faces_list)

        vertex_list = []
        # convert dictionary to an array suitable for transmission to
        # ctypes.
        for i in range(len(ordered_vertex)):
            # 3 elements per vertex
            vertex_list.append(ordered_vertex[i].co.x)
            vertex_list.append(ordered_vertex[i].co.y)
            vertex_list.append(ordered_vertex[i].co.z)


        c_vertex_buffer = (c_float * len(vertex_list))(*vertex_list)
        c_position = (c_float * 3)(*bmesh_tuple.position)
        c_rotation = (c_float * 3)(*bmesh_tuple.rotation)
        c_scale = (c_float * 3)(*bmesh_tuple.scale)
        c_name = c_char_p(bmesh_tuple.name.encode("ascii"))

        ret = self.render_girl_shared.AddSceneGroup(c_name,
                                              c_vertex_buffer, len(vertex_list),
                                              c_faces_buffer, len(faces_list),
                                              c_position, c_rotation, c_scale)

        return ret


    def clear_scene(self):
        " Clear all geometry loaded on the core "
        self.render_girl_shared.ClearScene()


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
