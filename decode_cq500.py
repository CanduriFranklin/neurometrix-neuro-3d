import pydicom, os, glob
from pydicom.uid import ExplicitVRLittleEndian

src = r"C:\Users\candu\source\repos\Mathematical_Neurometrix\data\curated\ct_head_test"
dst = r"C:\Users\candu\source\repos\Mathematical_Neurometrix\data\curated\ct_head_unc"
os.makedirs(dst, exist_ok=True)

n = 0
for f in sorted(glob.glob(os.path.join(src, "*.dcm"))):
    ds = pydicom.dcmread(f)
    arr = ds.pixel_array
    ds.PixelData = arr.tobytes()
    ds.file_meta.TransferSyntaxUID = ExplicitVRLittleEndian
    ds['PixelData'].is_undefined_length = False
    for tag in ('LossyImageCompression','LossyImageCompressionRatio','LossyImageCompressionMethod'):
        if tag in ds: del ds[tag]
    ds.save_as(os.path.join(dst, os.path.basename(f)), write_like_original=False)
    n += 1

print(f"Re-codificados: {n} -> {dst}")
