import pydicom, os
d = r"C:\Users\candu\source\repos\Mathematical_Neurometrix\data\curated\ct_head_test"
files = sorted(os.listdir(d))[:3]
for f in files:
    ds = pydicom.dcmread(os.path.join(d, f), stop_before_pixels=True)
    print(f"{f}: tsyntax={ds.file_meta.TransferSyntaxUID}  rows={ds.Rows} cols={ds.Columns} bits={ds.BitsAllocated} pixelsign={getattr(ds,'PixelRepresentation','?')}")
