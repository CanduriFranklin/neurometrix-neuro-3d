import zipfile, os, shutil

zp = r"C:\Users\candu\source\repos\Mathematical_Neurometrix\data\raw\cq500\qureai-headct.zip"
out_flat = r"C:\Users\candu\source\repos\Mathematical_Neurometrix\data\curated\ct_head_real"
series_prefix = "qct01/CQ500CT0 CQ500CT0/Unknown Study/CT 4cc sec 150cc D3D on-2/"

if os.path.exists(out_flat):
    shutil.rmtree(out_flat)
os.makedirs(out_flat, exist_ok=True)

n = 0
with zipfile.ZipFile(zp) as z:
    for name in z.namelist():
        if name.startswith(series_prefix) and name.lower().endswith(".dcm"):
            base = os.path.basename(name)
            with z.open(name) as src, open(os.path.join(out_flat, base), "wb") as dst:
                shutil.copyfileobj(src, dst)
            n += 1

print(f"DICOMs extraidos: {n} -> {out_flat}")
