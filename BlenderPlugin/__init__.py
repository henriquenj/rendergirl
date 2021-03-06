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


import bpy
import collections
from .Ui import *
from .RenderGirl import RenderGirl

from mathutils import Vector

# Metadata for blender internal plugin system
bl_info = {
    "name": "RenderGirl",
    "author": "Henrique Jung",
    "version": (0, 0, 1),
    "blender": (2, 76, 0),
    "location": "Info header, render engine menu",
    "description": "RenderGirl raytracer for Blender",
    "warning": "Still on very early stage of development!",
    "wiki_url": "https://github.com/henriquenj/rendergirl",
    "tracker_url": "",
    "support": 'TESTING',
    "category": "Render"}


class RenderGirlBlender(bpy.types.RenderEngine):
    """ This class provides interface between blender UI and the RenderGirl Core """
    bl_idname = 'RenderGirl'
    bl_label = 'RenderGirl'
    bl_use_preview = True

    def __init__(self):
        # update pointer of the static class with this session
        RenderGirl.instance.session = self

    def __del__(self):
        RenderGirl.instance.session = None

    def render(self, scene):
        scale = scene.render.resolution_percentage / 100.0
        size_x = int(scene.render.resolution_x * scale)
        size_y = int(scene.render.resolution_y * scale)

        pixel_count = size_x * size_y

        # loads all geometry on the the core
        # we are only interested in MESH objects for now
        objects = scene.objects

        # meshs list will contains a mesh object alongside its name,
        # position, rotation and scale
        MeshPlus = collections.namedtuple('MeshPlus',
                                ['mesh','name','position','scale','rotation'])
        meshs = []
        for i in range(len(objects)):
           if objects[i].type == 'MESH':
               # triangulate modifier will triangulate the mesh and be
               # deleted afterwards
               tri_modifier = objects[i].modifiers.new(
                   name="triangulate",type='TRIANGULATE')
               # convert to mesh objects (modifiers will be applied at this step)
               mesh = objects[i].to_mesh(scene,True,'RENDER')

               # Remark:
               # Blender uses XZY coordinate system, so we swap
               # the Y by Z where needed it.

               rot = Vector()
               # Rendergirl needs rotations to be XYZ ordered (XZY
               # from blender perspective), we get the Euler directly
               # from the matrix world passing the order we want on
               # the conversion.
               euler = objects[i].matrix_world.to_euler('XZY')
               rot.x = euler.x
               rot.y = euler.z
               rot.z = euler.y

               # extract position from matrix world
               pos = objects[i].matrix_world.translation.xzy

               mesh_tuple = MeshPlus(mesh,objects[i].name,
                                     pos,objects[i].scale.xzy, rot)
               meshs.append(mesh_tuple)
               objects[i].modifiers.remove(tri_modifier)

        # add to rendergirl core
        for i in range(len(meshs)):
            ret = RenderGirl.instance.add_scene_group(meshs[i])
            if ret != 0:
                raise ValueError("Error adding object {1} to RenderGirl"
                                 .format(meshs[i].name))

        light = None
        # grab first light found on the scene and use it to render
        # currently rendergirl supports only one light
        for i in range(len(objects)):
            if objects[i].type == 'LAMP':
                light = objects[i]
                break

        # objects set up, now render
        rect = RenderGirl.instance.render(size_x, size_y,
                                                    scene.camera, light)

        if rect == None:
            RenderGirl.instance.clear_scene()
            raise ValueError("Error rendering frame, please check the logs")

        # Here we write the pixel values to the RenderResult
        result = self.begin_result(0, 0, size_x, size_y)
        layer = result.layers[0]
        layer.passes[0].rect = rect
        self.end_result(result)

        # clear all geometry of this rendering
        # this will be removed as soon as we have a cache mechanism
        RenderGirl.instance.clear_scene()

def register():
    # create RenderGirl instance and start it
    RenderGirl()
    error = RenderGirl.instance.start()

    if error != 0:
        raise ValueError("Error starting rendergirl")

    # Register the RenderEngine
    bpy.utils.register_class(RenderGirlBlender)
    # TODO: consider using blender modules, more info here
    # http://www.blender.org/api/blender_python_api_2_64_release/info_overview.html#multiple-classes

    # Register the render properties class
    bpy.utils.register_class(RenderGirlRenderSettings)
    # register rendergirl own's post processing panel
    bpy.utils.register_class(RenderGirlPostProcessingPanel)

    # Register render properties UI elements
    from bl_ui import properties_render
    properties_render.RENDER_PT_render.COMPAT_ENGINES.add('RenderGirl')
    properties_render.RENDER_PT_dimensions.COMPAT_ENGINES.add('RenderGirl')
    # add function for drawning new buttons to the render panel
    properties_render.RENDER_PT_render.append(render_girl_render_options)

def unregister():
    RenderGirl.instance.finish()

    # Unregister render properties UI elements
    from bl_ui import properties_render
    properties_render.RENDER_PT_render.COMPAT_ENGINES.remove('RenderGirl')
    properties_render.RENDER_PT_dimensions.COMPAT_ENGINES.remove('RenderGirl')

    bpy.utils.unregister_class(RenderGirlBlender)
    bpy.utils.unregister_class(RenderGirlRenderSettings)
    bpy.utils.unregister_class(RenderGirlPostProcessingPanel)

    properties_render.RENDER_PT_render.remove(render_girl_render_options)
