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
from .RenderGirl import RenderGirl

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
        # update pointer instance pointer to the static class with
        # this session
        RenderGirlBlender.render_girl.session = self

    def __del__(self):
        RenderGirlBlender.render_girl.session = None

    def render(self, scene):
        scale = scene.render.resolution_percentage / 100.0
        self.size_x = int(scene.render.resolution_x * scale)
        self.size_y = int(scene.render.resolution_y * scale)

        pixel_count = self.size_x * self.size_y

        rect = []
        counter = 0
        color = 0.0

        while True:
            rect.append([ color, color, 0.5, 1.0])
            color += 0.1
            counter = counter + 1
            if color > 1:
                color = 0
            if counter == pixel_count:
                break

        # Here we write the pixel values to the RenderResult
        result = self.begin_result(0, 0, self.size_x, self.size_y)
        layer = result.layers[0]
        layer.rect = rect
        self.end_result(result)

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
