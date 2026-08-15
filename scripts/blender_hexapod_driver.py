"""
Example Blender script to drive an armature with the hexapod_core module.

Usage:
- Build the project so `build/hexapod_core*.so` exists.
- Edit `BUILD_DIR` below to point to your build directory if needed.
- In Blender: Text Editor -> Open this script -> Run Script.
- Ensure your armature object is named 'Armature' (or edit `ARMATURE_NAME`).
- Ensure bone names exist and are mapped in `BONE_MAP`.

Notes:
- Blender uses radians for `rotation_euler`.
- You may need to swap axes (X/Y/Z) in `AXIS_MAP` to match your rig.
- For running headless, run: `blender --background file.blend --python blender_hexapod_driver.py`
"""

import bpy
import sys
import os
import math

# === Configuration ===
# Set this to the build folder containing the compiled module (or copy the .pyd next to the .blend)
SCRIPT_DIR = os.path.abspath(os.path.dirname(__file__)) if '__file__' in globals() else os.getcwd()
BUILD_DIR = os.path.abspath(os.path.join(SCRIPT_DIR, '..', 'build'))
# Windows path where the native Blender Python extension is built.
WIN_BUILD_DIR = r'C:\Users\bened\hexapod_win\build'
WIN_BUILD_RELEASE_DIR = os.path.join(WIN_BUILD_DIR, 'Release')
# Windows UNC path to the WSL build output (Linux .so only; Blender on Windows cannot import it)
WSL_WINDOWS_BUILD_DIR = r'\\wsl$\\Ubuntu\\home\\ben\\hexapod\\build'
WSL_WINDOWS_BUILD_RELEASE_DIR = os.path.join(WSL_WINDOWS_BUILD_DIR, 'Release')
# If running inside Blender's Text Editor, replace BUILD_DIR with the absolute path to your build folder.

ARMATURE_NAME = 'Armature'
# Map logical leg keys to bone names (coxa, femur, tibia)
BONE_MAP = {
    'LF': ('LF_Coxa', 'LF_Femur', 'LF_Tibia'),
    'RF': ('RF_Coxa', 'RF_Femur', 'RF_Tibia'),
    'LM': ('LM_Coxa', 'LM_Femur', 'LM_Tibia'),
    'RM': ('RM_Coxa', 'RM_Femur', 'RM_Tibia'),
    'LB': ('LB_Coxa', 'LB_Femur', 'LB_Tibia'),
    'RB': ('RB_Coxa', 'RB_Femur', 'RB_Tibia'),
}
# Per-bone axis mapping (which Euler axis to write to). Adjust if your rig uses different local axes.
AXIS_MAP = {
    'coxa': 'Z',
    'femur': 'X',
    'tibia': 'X',
}
# Optional static offsets (radians) per bone if your rest pose differs
OFFSETS = { 'coxa': 0.0, 'femur': 0.0, 'tibia': 0.0 }

# === Motion mode selection ===
# Change this value to choose the behavior.
# Use one of: 'stand', 'walk_forward', 'walk_backward', 'turn_left', 'turn_right', 'turn_360', 'dance', 'stride'
MODE = 'turn_right'

# Per-mode tuning knobs.
# These are applied each frame before the motion method is called.
MOTION_SETTINGS = {
    'stand': {'speed': 0.8, 'step_length': 40.0, 'turn_rate': 0.4, 'dance_energy': 1.0},
    'walk_forward': {'speed': 0.8, 'step_length': 40.0, 'turn_rate': 0.4, 'dance_energy': 1.0},
    'walk_backward': {'speed': 0.8, 'step_length': 40.0, 'turn_rate': 0.4, 'dance_energy': 1.0},
    'turn_left': {'speed': 0.8, 'step_length': 35.0, 'turn_rate': 0.6, 'dance_energy': 1.0},
    'turn_right': {'speed': 0.8, 'step_length': 35.0, 'turn_rate': 0.6, 'dance_energy': 1.0},
    'turn_360': {'speed': 0.8, 'step_length': 35.0, 'turn_rate': 0.8, 'dance_energy': 1.0},
    'dance': {'speed': 1.2, 'step_length': 30.0, 'turn_rate': 0.4, 'dance_energy': 1.5},
    'stride': {'speed': 0.9, 'step_length': 55.0, 'turn_rate': 0.4, 'dance_energy': 1.0},
}

# === Import the compiled module ===
build_dirs = [
    BUILD_DIR,
    os.path.join(BUILD_DIR, 'Release'),
    WIN_BUILD_DIR,
    WIN_BUILD_RELEASE_DIR,
    WSL_WINDOWS_BUILD_DIR,
]
for p in build_dirs:
    if os.path.isdir(p) and p not in sys.path:
        sys.path.insert(0, p)

try:
    import hexapod_core
except Exception as e:
    raise ImportError(f"Failed to import hexapod_core from {build_dirs}: {e}")

# Verify that the loaded module matches the current Python API.
required_methods = ['setWalkSpeed', 'setStepLength', 'setTurnRate', 'setDanceEnergy']
probe = hexapod_core.Hexapod(1.0, 1.0, 1.0)
missing_methods = [name for name in required_methods if not hasattr(probe, name)]
if missing_methods:
    raise RuntimeError(
        f"Loaded module '{getattr(hexapod_core, '__file__', '<unknown>')}' is outdated. "
        f"Missing methods: {missing_methods}. Rebuild the extension and reload Blender."
    )
print(f"Loaded hexapod_core from: {hexapod_core.__file__}")

# Create hexapod instance
h = hexapod_core.Hexapod(17.0, 55.0, 75.0)

# Apply the current tuning values to the C++ controller.
def apply_motion_settings(mode_name):
    settings = MOTION_SETTINGS[mode_name]
    h.setWalkSpeed(settings['speed'])
    h.setStepLength(settings['step_length'])
    h.setTurnRate(settings['turn_rate'])
    h.setDanceEnergy(settings['dance_energy'])

apply_motion_settings(MODE)

# Helper to write an angle to a bone's euler rotation
def set_bone_angle(pose_bone, axis, angle_rad):
    pose_bone.rotation_mode = 'XYZ'
    # copy current euler to avoid wiping other axes
    e = list(pose_bone.rotation_euler)
    idx = {'X':0, 'Y':1, 'Z':2}[axis]
    e[idx] = angle_rad
    pose_bone.rotation_euler = e

# Frame-change handler
def hexapod_frame_handler(scene):
    # time in seconds derived from frame and fps
    t = scene.frame_current / scene.render.fps

    # Re-apply the selected mode every frame so you can edit MODE manually.
    apply_motion_settings(MODE)

    if MODE == 'stand':
        h.stand(t)
    elif MODE == 'walk_forward':
        h.walkForward(t)
    elif MODE == 'walk_backward':
        h.walkBackward(t)
    elif MODE == 'turn_left':
        h.turnLeft(t)
    elif MODE == 'turn_right':
        h.turnRight(t)
    elif MODE == 'turn_360':
        h.turn360(t)
    elif MODE == 'dance':
        h.dance(t)
    elif MODE == 'stride':
        h.stride(t)
    else:
        h.stand(t)

    arm = bpy.data.objects.get(ARMATURE_NAME)
    if arm is None:
        print(f"Armature '{ARMATURE_NAME}' not found")
        return

    angles = {
        'LF': h.getFL(), 'RF': h.getFR(),
        'LM': h.getML(), 'RM': h.getMR(),
        'LB': h.getBL(), 'RB': h.getBR(),
    }

    for leg_key, bones in BONE_MAP.items():
        ang = angles[leg_key]
        # Map values
        bone_coxa, bone_femur, bone_tibia = bones
        try:
            pb_coxa = arm.pose.bones[bone_coxa]
            pb_femur = arm.pose.bones[bone_femur]
            pb_tibia = arm.pose.bones[bone_tibia]
        except KeyError:
            # bone missing
            continue

        set_bone_angle(pb_coxa, AXIS_MAP['coxa'], ang.coxa + OFFSETS['coxa'])
        set_bone_angle(pb_femur, AXIS_MAP['femur'], ang.femur + OFFSETS['femur'])
        set_bone_angle(pb_tibia, AXIS_MAP['tibia'], ang.tibia + OFFSETS['tibia'])

    # update the viewport
    bpy.context.view_layer.update()


# Register the handler (avoid duplicates)
handler = hexapod_frame_handler
if handler not in bpy.app.handlers.frame_change_pre:
    bpy.app.handlers.frame_change_pre.append(handler)

print('hexapod driver registered')
