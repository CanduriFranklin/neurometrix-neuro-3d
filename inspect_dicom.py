import pydicom
import os
from collections import defaultdict

root = r"C:\Users\candu\source\repos\Mathematical_Neurometrix\data\curated\real"

groups = defaultdict(list)
errors = []

for name in sorted(os.listdir(root)):
    if not name.lower().endswith(".dcm"):
        continue
    path = os.path.join(root, name)
    try:
        ds = pydicom.dcmread(path, stop_before_pixels=True)
        key = (
            str(ds.get("Modality", "?")),
            str(ds.get("BodyPartExamined", "")),
            int(ds.get("Rows", 0)),
            int(ds.get("Columns", 0)),
            str(ds.get("StudyInstanceUID", "")),
            str(ds.get("SeriesInstanceUID", "")),
        )
        groups[key].append(name)
    except Exception as e:
        errors.append((name, str(e)[:70]))

print(f"Archivos .dcm: {sum(len(v) for v in groups.values()) + len(errors)}")
print(f"Grupos unicos: {len(groups)}")
print(f"Errores lectura: {len(errors)}")
print()

print("=== Grupos con >= 4 slices ===")
for key, files in sorted(groups.items(), key=lambda x: -len(x[1])):
    if len(files) < 4:
        continue
    mod, body, rows, cols, study, series = key
    print(f"[{len(files):3d}] mod={mod:4s} body={body:14s} dims={rows}x{cols}  series=...{series[-24:]}")
    for f in files[:8]:
        print(f"        {f}")
    if len(files) > 8:
        print(f"        ... +{len(files)-8} mas")
    print()

print("=== Grupos con < 4 slices ===")
for key, files in sorted(groups.items(), key=lambda x: -len(x[1])):
    if len(files) >= 4:
        continue
    mod, body, rows, cols, study, series = key
    print(f"[{len(files):3d}] mod={mod:4s} body={body:14s} dims={rows}x{cols}  {files[:3]}")

print()
print("=== Errores ===")
for n, e in errors[:20]:
    print(f"  {n}: {e}")
