import struct
from kafka import KafkaProducer
mid = 700002
study_uid = "1.2.826.0.1.3680043.10.999.CQ500.CT0.subset48.unc"
dicom = "C:/Users/candu/source/repos/Mathematical_Neurometrix/data/curated/ct_head_unc"
ub = study_uid.encode(); pb = dicom.encode()
payload = struct.pack("<Q", mid) + struct.pack("<Q", 1735693000000) + struct.pack("<I", len(ub)) + ub + struct.pack("<I", len(pb)) + pb
p = KafkaProducer(bootstrap_servers="localhost:9092", acks="all")
m = p.send("core-3d.requests", value=payload).get(timeout=10)
print(f"OK offset={m.offset}")
p.flush(); p.close()
