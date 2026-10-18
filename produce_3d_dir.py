import struct
from kafka import KafkaProducer

mid       = 500011
ts        = 1735689700000
study_uid = "1.2.840.113619.2.55.3.STUDY_DEMO_3D_002"
dicom     = "C:/Users/candu/source/repos/Mathematical_Neurometrix/data/curated/real"

uid_b  = study_uid.encode("utf-8")
path_b = dicom.encode("utf-8")

payload = (
    struct.pack("<Q", mid) +
    struct.pack("<Q", ts) +
    struct.pack("<I", len(uid_b)) + uid_b +
    struct.pack("<I", len(path_b)) + path_b
)

p = KafkaProducer(bootstrap_servers="localhost:9092", acks="all")
meta = p.send("core-3d.requests", value=payload).get(timeout=10)
print(f"OK  topic={meta.topic}  partition={meta.partition}  offset={meta.offset}")
p.flush()
p.close()
