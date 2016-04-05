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

from mathutils import Vector

class RenderGirl:
    """Class that loads the RenderGirl shared library and holds most of
    the wrappers. The difference between RenderGirl and
    RenderGirlBlender is that RenderGirl class is a singleton that is
    not suppose to be deleted across several renderings. So it's
    useful to retain states and perform init operations.

    """

    instance = None

    def __init__(self):
        # the current instance of RenderGirlBlender running
        self.session = None
        self.device_selected = False
        RenderGirl.instance = self


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

        # fetch device amount and device names
        devices_amount = int(self.render_girl_shared.FetchDevicesSize())

        strings_ptr = (c_char_p * devices_amount)()
        self.render_girl_shared.FetchDevicesName(byref(strings_ptr))

        self.device_names = []
        for a in range(0, devices_amount):
            # fill list with devices name
            name = str(strings_ptr[a].decode("ascii"))
            self.device_names.append(name)


        print("RenderGirl started")
        return 0

    def add_scene_group(self, mesh_tuple):
        """Add an object to RenderGirl scene
        @param mesh_tuple namedtuple composed of a tringulated mesh
        alongside its name, position, scale and rotation
        @return 0 for success, -1 for error
        """
        if mesh_tuple == None:
            return -1

        faces_list = [] # each face is composed of a list with three
                        # integer that points to the vertex list
        vertex_list = [] # each vertex is composed of a list with
                         # three float numbers representing XYZ

        # blender internal polygons list
        polygons_list = mesh_tuple.mesh.polygons.values()
        # iterate over polygons grabbing indexes of each vertex.  all
        # polygons are already triangulated, so every polygon has
        # three vertices
        for polygon in polygons_list:
            faces_list.append(polygon.vertices[0])
            faces_list.append(polygon.vertices[2])
            faces_list.append(polygon.vertices[1])
            # faces must be swapped otherwise normals get screwde
            # TODO: investigate it further


        # list of bpy.types.MeshVertex
        mesh_vert = mesh_tuple.mesh.vertices.values()
        # convert dictionary to an array suitable for transmission to
        # ctypes.
        for i in range(len(mesh_vert)):
            # 3 elements per vertex
            vertex_list.append(mesh_vert[i].co.x)
            vertex_list.append(mesh_vert[i].co.z)
            vertex_list.append(mesh_vert[i].co.y)
            # blender uses Z as height


        # create Ctypes buffers objects

        # the faces look like this
        # faces_list = [ 1, 2, 4 , 4, 5, 6 , 6, 4, 3 , ... ]
        # each of them points to a position in vertex_list
        c_faces_buffer = (c_int * len(faces_list))(*faces_list)
        # vertex_list looks like this [0.0 , 1.0 , 1.5, 2.0 , 1.0 , 2.0 , ... ]
        c_vertex_buffer = (c_float * len(vertex_list))(*vertex_list)
        c_position = (c_float * 3)(*mesh_tuple.position)
        c_rotation = (c_float * 3)(*mesh_tuple.rotation)
        c_scale = (c_float * 3)(*mesh_tuple.scale)
        c_name = c_char_p(mesh_tuple.name.encode("ascii"))

        ret = self.render_girl_shared.AddSceneGroup(c_name,
                                              c_vertex_buffer, len(vertex_list),
                                              c_faces_buffer, len(faces_list),
                                              c_position, c_rotation, c_scale)

        return ret


    def render(self, width, height, camera, light):
        """ Render a frmae using the currently loaded objects on the core
        @param width width of the frame in pixels
        @param height height of the frame in pixels
        @param camera camera to be used on rendering
        @param light object representing the light (only one supported so far)
        @return list with height * width pixels
        """

        pixel_count = width * height

        # extract positions from world matrix
        cam_pos = camera.matrix_world.translation.xzy
        light_pos = light.matrix_world.translation.xzy

        quat = camera.matrix_world.to_quaternion()
        # compute direction an up vector
        cam_up = quat * Vector((0.0 , 1.0 , 0.0))
        cam_dir = quat * Vector((0.0 , 0.0 , -1.0))

        # get light color
        light_color = [light.data.color.r, light.data.color.g, light.data.color.b]

        # C types
        c_cam_pos = (c_float * 3)(*cam_pos)
        c_cam_up = (c_float * 3)(*cam_up.xzy)
        c_cam_dir = (c_float * 3)(*cam_dir.xzy)
        c_light_pos = (c_float * 3)(*light_pos)
        c_light_color = (c_float * 3)(*light_color)

        # alloc the frame buffer
        c_frame_size = pixel_count * 4
        c_out_frame = (c_ubyte * c_frame_size)()

        ret = self.render_girl_shared.Render(width, height, c_cam_pos,
                                             c_cam_up, c_cam_dir,
                                             c_light_pos,
                                             c_light_color,
                                             byref(c_out_frame))

        if ret == -1:
            return None

        frame = []
        # copy frame to format accepted by blender
        for i in range(0,c_frame_size,4):
            pixel = []
            pixel.append(c_out_frame[i] / 256.0) # red
            pixel.append(c_out_frame[i + 1] / 256.0) # green
            pixel.append(c_out_frame[i + 2] / 256.0) # blue
            pixel.append(c_out_frame[i + 3] / 256.0) # alpha
            frame.append(pixel)

        return frame



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
