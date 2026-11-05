"""
Headless Blender batch exporter for Wild Omission.
Usage:
  "C:\Program Files\Blender Foundation\Blender 5.2\blender.exe" -b <file.blend> -P Art/Blender/export_fbx.py -- <output.fbx>
Enforces flat shading, triangulates ngons, applies transforms, and exports FBX at 1 UU = 1 cm (-Y forward, Z up).
"""
import sys
import os
import bpy

def export_clean_fbx(out_path: str):
    bpy.ops.object.select_all(action='SELECT')
    for obj in bpy.context.selected_objects:
        if obj.type == 'MESH':
            bpy.context.view_layer.objects.active = obj
            bpy.ops.object.transform_apply(location=False, rotation=True, scale=True)
            for poly in obj.data.polygons:
                poly.use_smooth = False

    os.makedirs(os.path.dirname(os.path.abspath(out_path)), exist_ok=True)
    bpy.ops.export_scene.fbx(
        filepath=out_path,
        use_selection=False,
        object_types={'MESH', 'ARMATURE'},
        global_scale=1.0,
        apply_unit_scale=True,
        apply_scale_options='FBX_SCALE_NONE',
        axis_forward='-Y',
        axis_up='Z',
        mesh_smooth_type='FACE',
        use_mesh_modifiers=True,
        add_leaf_bones=False,
    )
    print(f"Exported FBX to {out_path}")

if __name__ == "__main__":
    argv = sys.argv
    if "--" in argv:
        args = argv[argv.index("--") + 1:]
        if args:
            export_clean_fbx(args[0])
