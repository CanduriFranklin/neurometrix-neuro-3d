import os, numpy as np
import pydicom
from pydicom.dataset import FileDataset, FileMetaDataset
from pydicom.uid import generate_uid, ExplicitVRLittleEndian, CTImageStorage

out = r"C:\Users\candu\source\repos\Mathematical_Neurometrix\data\curated\synthetic_3d"
os.makedirs(out, exist_ok=True)

study_uid  = "1.2.826.0.1.3680043.10.999.SYNTH3D.001"
series_uid = "1.2.826.0.1.3680043.10.999.SYNTH3D.001.1"
N, rows, cols = 8, 128, 128
yy, xx = np.mgrid[0:rows, 0:cols]
cy, cx, rb = rows/2, cols/2, 45

for i in range(N):
    arr = np.full((rows, cols), -1000, dtype=np.int32)
    arr[(yy-cy)**2 + (xx-cx)**2 < rb**2] = 40
    arr[(yy-cy)**2 + (xx-cx)**2 < 12**2] = 10
    sk = ((yy-cy)**2 + (xx-cx)**2 >= rb**2) & ((yy-cy)**2 + (xx-cx)**2 < (rb+3)**2)
    arr[sk] = 900
    if 2 <= i <= 4:
        s = (yy-(cy-10))**2 + (xx-(cx+8))**2 < 4**2
        arr[s] = 65
    if 3 <= i <= 5:
        arr[int(cy)+15:int(cy)+18, int(cx)-20:int(cx)+20] = 1200
    raw = (arr + 1024).astype(np.int16)

    fm = FileMetaDataset()
    fm.MediaStorageSOPClassUID = CTImageStorage
    fm.MediaStorageSOPInstanceUID = generate_uid()
    fm.TransferSyntaxUID = ExplicitVRLittleEndian
    fm.ImplementationClassUID = generate_uid()

    ds = FileDataset(None, {}, file_meta=fm, preamble=b"\0"*128)
    ds.SOPClassUID = CTImageStorage
    ds.SOPInstanceUID = fm.MediaStorageSOPInstanceUID
    ds.StudyInstanceUID = study_uid
    ds.SeriesInstanceUID = series_uid
    ds.Modality = "CT"; ds.BodyPartExamined = "HEAD"
    ds.PatientName = "SYNTHETIC^3D"; ds.PatientID = "SYN001"
    ds.StudyDate = "20260101"; ds.StudyTime = "120000"
    ds.SeriesNumber = 1; ds.InstanceNumber = i + 1
    ds.SliceLocation = float(i * 2.0)
    ds.ImagePositionPatient = [0.0, 0.0, float(i * 2.0)]
    ds.ImageOrientationPatient = [1,0,0,0,1,0]
    ds.PixelSpacing = [1.0, 1.0]; ds.SliceThickness = 2.0
    ds.Rows = rows; ds.Columns = cols
    ds.SamplesPerPixel = 1; ds.PhotometricInterpretation = "MONOCHROME2"
    ds.BitsAllocated = 16; ds.BitsStored = 16; ds.HighBit = 15
    ds.PixelRepresentation = 1
    ds.RescaleIntercept = -1024; ds.RescaleSlope = 1
    ds.WindowCenter = 40; ds.WindowWidth = 400
    ds.PixelData = raw.tobytes()
    ds.save_as(os.path.join(out, f"synth_{i:03d}.dcm"))

print(f"OK: {N} slices -> {out}")
print(f"study_uid = {study_uid}")
