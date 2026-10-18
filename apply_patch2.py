path = r"C:\Users\candu\source\repos\Mathematical_Neurometrix\neuro-3d\application\use_cases\process_volume_use_case.cpp"
with open(path, 'r', encoding='utf-8') as f:
    txt = f.read()

# Solo el bloque de vesselness: bajar techo a 400 HU (excluye cráneo)
old = """        const double hu_i = static_cast<double>(vol.hu()(i));
        if (hu_i < -200.0 || hu_i > 2000.0) continue;
        if (vesselness(i) > v_peak) { v_peak = vesselness(i); v_idx = i; }"""

new = """        const double hu_i = static_cast<double>(vol.hu()(i));
        if (hu_i < -200.0 || hu_i > 400.0) continue;
        if (vesselness(i) > v_peak) { v_peak = vesselness(i); v_idx = i; }"""

assert old in txt, "bloque vesselness no encontrado"
txt = txt.replace(old, new)

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(txt)
print("FIX vesselness OK")
