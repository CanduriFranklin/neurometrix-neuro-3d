path = r"C:\Users\candu\source\repos\Mathematical_Neurometrix\neuro-3d\application\use_cases\process_volume_use_case.cpp"

with open(path, 'r', encoding='utf-8') as f:
    txt = f.read()

old1 = """    double th_peak = 0.0;
    int th_idx = 0;
    for (int i = 0; i < static_cast<int>(tophat.rows()); ++i) {
        if (tophat(i) > th_peak) { th_peak = tophat(i); th_idx = i; }
    }
    if (th_peak > 25.0) {"""

new1 = """    double th_peak = 0.0;
    int th_idx = -1;
    for (int i = 0; i < static_cast<int>(tophat.rows()); ++i) {
        const double hu_i = static_cast<double>(vol.hu()(i));
        if (hu_i < -200.0 || hu_i > 2000.0) continue;
        if (tophat(i) > th_peak) { th_peak = tophat(i); th_idx = i; }
    }
    if (th_idx >= 0 && th_peak > 25.0) {"""

old2 = """    double v_peak = 0.0;
    int v_idx = 0;
    for (int i = 0; i < static_cast<int>(vesselness.rows()); ++i) {
        if (vesselness(i) > v_peak) { v_peak = vesselness(i); v_idx = i; }
    }
    if (v_peak > 0.15) {"""

new2 = """    double v_peak = 0.0;
    int v_idx = -1;
    for (int i = 0; i < static_cast<int>(vesselness.rows()); ++i) {
        const double hu_i = static_cast<double>(vol.hu()(i));
        if (hu_i < -200.0 || hu_i > 2000.0) continue;
        if (vesselness(i) > v_peak) { v_peak = vesselness(i); v_idx = i; }
    }
    if (v_idx >= 0 && v_peak > 0.15) {"""

assert old1 in txt, "bloque 1 no encontrado"
assert old2 in txt, "bloque 2 no encontrado"

txt = txt.replace(old1, new1).replace(old2, new2)

with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(txt)

print("PATCH OK")
