# RenderGirl - OpenCL raytracer renderer
# Copyright (c) 2016, Henrique Jung, All rights reserved.

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
from bpy.props import (BoolProperty,
                       EnumProperty,
                       FloatProperty,
                       IntProperty,
                       PointerProperty)
from .RenderGirl import RenderGirl


def ocl_devices(scene, context):
    """ Fetch devices list from RenderGirl """
    devices = []

    rgirl_devices = RenderGirl.instance.device_names

    for name in rgirl_devices:
        name_tupple = (name,name,"")
        devices.append(name_tupple)

    return devices


class RenderGirlRenderSettings(bpy.types.PropertyGroup):
    """ Class to register and controls the RenderGirl properties within Blender
    """

    @classmethod
    def register(cls):

        bpy.types.Scene.rgirl_settings = PointerProperty(
        name = "RenderGirl render settings",
        description = "RenderGirl render settings properties",
        type=RenderGirlRenderSettings)

        cls.device = EnumProperty(
                name="Device",
                description="Device to use for rendering",
                items=ocl_devices,
                )

    @classmethod
    def unregister(cls):
        del bpy.types.Scene.rgirl_settings




def render_girl_render_options(self, context):
    """ Draw function to add options to the render panel"""

    # we don't draw unless we have rendergirl selected
    if context.scene.render.engine == 'RenderGirl':
        self.layout.prop(context.scene.rgirl_settings,"device")
