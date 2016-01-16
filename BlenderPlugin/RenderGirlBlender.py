import bpy

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


class RenderGirl(bpy.types.RenderEngine):
    """ This class provides interface between blender UI and the RenderGirl Core """
    bl_idname = 'RenderGirl'
    bl_label = 'RenderGirl'
    bl_use_preview = True

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
    # Register the RenderEngine
    bpy.utils.register_class(RenderGirl)
    # TODO: consider using blender modules, more info here
    # http://www.blender.org/api/blender_python_api_2_64_release/info_overview.html#multiple-classes

    # Register render properties UI elements
    from bl_ui import properties_render
    properties_render.RENDER_PT_render.COMPAT_ENGINES.add('RenderGirl')
    del properties_render


def unregister():
    bpy.utils.unregister_class(RenderGirl)
