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
from .RenderGirl import RenderGirl

from mathutils import Vector

# Metadata for blender internal plugin system
bl_info = {
    "name": "RenderGirl",
    "author": "Henrique Jung",
    "version": (0, 0, 1),
    "blender": (2, 70, 0), # TODO(Henrique Jung): test with other versions of blender
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
    render_girl = None

    def __init__(self):
        # update pointer of the static class with this session
        RenderGirlBlender.render_girl.session = self

    def __del__(self):
        RenderGirlBlender.render_girl.session = None

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
               # position is held within matrix_world, we just extract
               # it. Blender uses XZY coordinate system, so we swap
               # the positions where needed it.
               rot = Vector()
               rot.x = objects[i].rotation_euler.x
               rot.y = objects[i].rotation_euler.z
               rot.z = objects[i].rotation_euler.y
               pos = objects[i].matrix_world.translation.xzy
               mesh_tuple = MeshPlus(mesh,objects[i].name,
                                     pos,objects[i].scale.xzy, rot)
               meshs.append(mesh_tuple)
               objects[i].modifiers.remove(tri_modifier)

        # add to rendergirl core
        for i in range(len(meshs)):
            ret = RenderGirlBlender.render_girl.add_scene_group(meshs[i])
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
        rect = RenderGirlBlender.render_girl.render(size_x, size_y,
                                                    scene.camera, light)

        if rect == None:
            RenderGirlBlender.render_girl.clear_scene()
            raise ValueError("Error rendering frame, please check the logs")

        # Here we write the pixel values to the RenderResult
        result = self.begin_result(0, 0, size_x, size_y)
        layer = result.layers[0]
        layer.rect = rect
        self.end_result(result)

        # clear all geometry of this rendering
        # this will be removed as soon as we have a cache mechanism
        RenderGirlBlender.render_girl.clear_scene()

def register():
    # create RenderGirl instance and start it
    RenderGirlBlender.render_girl = RenderGirl()
    error = RenderGirlBlender.render_girl.start()

    if error != 0:
        raise ValueError("Error starting rendergirl")

    # Register the RenderEngine
    bpy.utils.register_class(RenderGirlBlender)
    # TODO: consider using blender modules, more info here
    # http://www.blender.org/api/blender_python_api_2_64_release/info_overview.html#multiple-classes

    # Register render properties UI elements
    from bl_ui import properties_render
    properties_render.RENDER_PT_render.COMPAT_ENGINES.add('RenderGirl')
    del properties_render


def unregister():
    bpy.utils.unregister_class(RenderGirlBlender)
    RenderGirlBlender.render_girl.finish()
    RenderGirlBlender.render_girl = None
