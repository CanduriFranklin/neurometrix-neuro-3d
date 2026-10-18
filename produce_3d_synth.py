import struct
from kafka import KafkaProducer

mid       = 500021
ts        = 1735690000000
study_uid = "1.2.826.0.1.3680043.10.999.SYNTH3D.001"
dicom     = "C:/Users/candu/source/repos/Mathematical_Neurometrix/data/curated/synthetic_3d"

uid_b  = study_uid.encode("utf-8")
path_b = dicom.encode("utf-8")
payload = (
    struct.pack("<Q", mid) + struct.pack("<Q", ts) +
    struct.pack("<I", len(uid_b)) + uid_b +
    struct.pack("<I", len(path_b)) + path_b
)
p = KafkaProducer(bootstrap_servers="localhost:9092", acks="all")
meta = p.send("core-3d.requests", value=payload).get(timeout=10)
print(f"OK  offset={meta.offset}")
p.flush(); p.close()
