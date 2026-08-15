#!/usr/bin/env python3
import sys, os, math

# Ensure the built module in ../build or ../build/Release is importable
build_base = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', 'build'))
wsld = r'\\wsl$\\Ubuntu\\home\\ben\\hexapod\\build'
build_dirs = [
    build_base,
    os.path.join(build_base, 'Release'),
    wsld,
    os.path.join(wsld, 'Release'),
]
for p in build_dirs:
    if os.path.isdir(p) and p not in sys.path:
        sys.path.insert(0, p)

try:
    import hexapod_core
except Exception as e:
    print('Failed to import hexapod_core:', e)
    print('Searched paths:', build_dirs)
    sys.exit(1)

h = hexapod_core.Hexapod(17.0, 55.0, 75.0)

def deg(rad):
    return rad * 180.0 / math.pi

def print_leg(name, ang):
    print(f"{name}: coxa={deg(ang.coxa):.2f} deg  femur={deg(ang.femur):.2f} deg  tibia={deg(ang.tibia):.2f} deg")

if __name__ == '__main__':
    times = [0.0, 0.5, 1.0, 1.5, 2.0]
    for t in times:
        print(f"--- time {t} ---")
        h.updateAllLegs(t)
        print_leg('FL', h.getFL())
        print_leg('FR', h.getFR())
        print_leg('ML', h.getML())
        print_leg('MR', h.getMR())
        print_leg('BL', h.getBL())
        print_leg('BR', h.getBR())
        print()
