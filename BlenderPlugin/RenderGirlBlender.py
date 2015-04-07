import bpy

# TODO (Henrique Jung): added tags for proper identification inside blender

class RenderGirl(bpy.types.RenderEngine):
    """ This class provides interface between blender interface and the RenderGirl Core """
    bl_idname = 'rendergirl'
    bl_label = 'RenderGirl'
    bl_use_preview = True

    def render(self, scene):
        scale = scene.render.resolution_percentage / 100.0
        self.size_x = int(scene.render.resolution_x * scale)
        self.size_y = int(scene.render.resolution_y * scale)



# Register the RenderEngine
bpy.utils.register_class(RenderGirl)

# Register render properties UI elements
from bl_ui import properties_render
properties_render.RENDER_PT_render.COMPAT_ENGINES.add('rendergirl')
del properties_render
