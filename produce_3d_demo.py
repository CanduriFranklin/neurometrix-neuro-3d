import struct
from kafka import KafkaProducer

mid       = 500010
ts        = 1735689600000
study_uid = "1.2.840.113619.2.55.3.STUDY_DEMO_3D_001"
dicom     = "C:/Users/candu/source/repos/Mathematical_Neurometrix/data/curated/3d/00000013_005_3d.dcm"

uid_b  = study_uid.encode("utf-8")
path_b = dicom.encode("utf-8")

payload = (
    struct.pack("<Q", mid) +
    struct.pack("<Q", ts) +
    struct.pack("<I", len(uid_b)) + uid_b +
    struct.pack("<I", len(path_b)) + path_b
)

print(f"payload = {len(payload)} bytes; uid={len(uid_b)}, path={len(path_b)}")

p = KafkaProducer(bootstrap_servers="localhost:9092", acks="all")
meta = p.send("core-3d.requests", value=payload).get(timeout=10)
print(f"OK  topic={meta.topic}  partition={meta.partition}  offset={meta.offset}")
p.flush()
p.close()
