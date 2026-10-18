import pydicom, os
from collections import defaultdict

roots = [
    r"C:\Users\candu\source\repos\Mathematical_Neurometrix\data",
]
groups = defaultdict(list)
errors = []

for root in roots:
    for dirpath, _, files in os.walk(root):
        for name in files:
            if not name.lower().endswith(".dcm"):
                continue
            path = os.path.join(dirpath, name)
            try:
                ds = pydicom.dcmread(path, stop_before_pixels=True)
                key = (
                    os.path.relpath(dirpath, root),
                    str(ds.get("Modality", "?")),
                    str(ds.get("BodyPartExamined", "")),
                    int(ds.get("Rows", 0)),
                    int(ds.get("Columns", 0)),
                    str(ds.get("StudyInstanceUID", "")),
                    str(ds.get("SeriesInstanceUID", "")),
                )
                groups[key].append(name)
            except Exception as e:
                errors.append((path, str(e)[:60]))

print(f"Total DICOMs: {sum(len(v) for v in groups.values()) + len(errors)}")
print(f"Grupos unicos: {len(groups)}")
print(f"Errores: {len(errors)}")
print()

print("=== Grupos con >= 4 slices (CT / HEAD o BRAIN) ===")
for key, files in sorted(groups.items(), key=lambda x: -len(x[1])):
    folder, mod, body, rows, cols, study, series = key
    if len(files) < 4: continue
    if mod != "CT": continue
    marker = " <<< CANDIDATO" if body in ("HEAD","BRAIN","SKULL","NEURO","") else ""
    print(f"[{len(files):3d}] {folder}  mod={mod} body={body} dims={rows}x{cols}{marker}")
    for f in files[:4]: print(f"       {f}")
    if len(files) > 4: print(f"       ... +{len(files)-4}")
    print()

print("=== TODOS los grupos >= 4 (cualquier modalidad) ===")
for key, files in sorted(groups.items(), key=lambda x: -len(x[1])):
    folder, mod, body, rows, cols, study, series = key
    if len(files) < 4: continue
    print(f"[{len(files):3d}] {folder:30s} mod={mod:5s} body={body:12s} dims={rows}x{cols}")

print()
print("=== Grupos < 4 (resumen) ===")
small = defaultdict(int)
for key, files in groups.items():
    if len(files) >= 4: continue
    folder, mod, body, rows, cols, study, series = key
    small[(folder, mod)] += len(files)
for (folder, mod), n in sorted(small.items()):
    print(f"  {folder:30s} mod={mod:5s} total={n}")
