"""
Generates a flat-shaded low-poly Metal Ore Node (SM_MetalOreNode) matching Wild Omission's art style:
- Untextured faceted icosphere-based boulder (~120 tris) with protruding metallic ore veins
- Uses exact colors from Art/Palette.json (Stone_Medium #6E7378 and Metal_Rusted #8C4A2F / Metal_HQM #A8B4C0)
"""
import json
import os
import random
import bpy
import bmesh
from mathutils import Vector

def hex_to_rgba(hex_str: str):
    h = hex_str.lstrip('#')
    return tuple(int(h[i:i+2], 16) / 255.0 for i in (0, 2, 4)) + (1.0,)

def make_mat(name: str, hex_color: str, metallic: float = 0.0, roughness: float = 0.85):
    mat = bpy.data.materials.new(name=name)
    mat.use_nodes = True
    bsdf = mat.node_tree.nodes.get("Principled BSDF")
    if bsdf:
        bsdf.inputs["Base Color"].default_value = hex_to_rgba(hex_color)
        bsdf.inputs["Metallic"].default_value = metallic
        bsdf.inputs["Roughness"].default_value = roughness
    return mat

def main():
    bpy.ops.wm.read_factory_settings(use_empty=True)
    repo_root = os.path.abspath(os.path.join(os.path.dirname(__file__), "..", ".."))
    with open(os.path.join(repo_root, "Art", "Palette.json"), "r", encoding="utf-8-sig") as f:
        palette = json.load(f)["colors"]

    mat_stone = make_mat("M_Stone_Medium", palette["Stone_Medium"], metallic=0.0, roughness=0.9)
    mat_ore = make_mat("M_Metal_Rusted", palette["Metal_Rusted"], metallic=0.35, roughness=0.65)

    # Create base boulder
    bpy.ops.mesh.primitive_ico_sphere_add(subdivisions=2, radius=75.0, location=(0.0, 0.0, 45.0))
    boulder = bpy.context.active_object
    boulder.name = "SM_MetalOreNode"

    random.seed(42)
    bm = bmesh.new()
    bm.from_mesh(boulder.data)
    for v in bm.verts:
        # Flatten bottom and displace vertices for faceted low-poly rock silhouette
        if v.co.z < -25.0:
            v.co.z = -45.0
        else:
            scale = random.uniform(0.82, 1.18)
            v.co.x *= scale
            v.co.y *= scale
            v.co.z *= random.uniform(0.85, 1.12)

    boulder.data.materials.append(mat_stone)
    boulder.data.materials.append(mat_ore)

    for face in bm.faces:
        face.smooth = False
        # Assign upper angled facets as protruding ore veins
        if face.calc_center_median().z > 0.0 and random.random() < 0.28:
            face.material_index = 1
        else:
            face.material_index = 0

    bm.to_mesh(boulder.data)
    bm.free()

    blend_path = os.path.join(repo_root, "Art", "Blender", "SM_MetalOreNode.blend")
    fbx_path = os.path.join(repo_root, "Art", "Export", "SM_MetalOreNode.fbx")
    os.makedirs(os.path.dirname(fbx_path), exist_ok=True)
    bpy.ops.wm.save_as_mainfile(filepath=blend_path)

    # Export via our standard exporter
    import sys
    sys.path.insert(0, os.path.dirname(__file__))
    import export_fbx
    export_fbx.export_clean_fbx(fbx_path)

if __name__ == "__main__":
    main()
