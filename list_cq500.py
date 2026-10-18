import zipfile
zp = r"C:\Users\candu\source\repos\Mathematical_Neurometrix\data\raw\cq500\qureai-headct.zip"

with zipfile.ZipFile(zp) as z:
    names = z.namelist()

print(f"Total entries: {len(names)}")
print(f"Primeros 40 paths:")
for n in names[:40]:
    print(" ", n)

# Niveles de anidado
import os
tops = set()
for n in names:
    parts = n.split("/")
    if len(parts) >= 1 and parts[0]:
        tops.add(parts[0])

print(f"\nTop-level dirs ({len(tops)}):")
for t in sorted(tops)[:20]:
    print(" ", t)

# Contar dcm por carpeta nivel-2
from collections import Counter
c = Counter()
for n in names:
    if n.lower().endswith(".dcm"):
        parts = n.split("/")
        if len(parts) >= 3:
            c["/".join(parts[:2])] += 1

print(f"\nCarpetas nivel-2 con mas DICOMs (top 10):")
for k, v in c.most_common(10):
    print(f"  [{v:5d}] {k}")
