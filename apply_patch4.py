path = r"C:\Users\candu\source\repos\Mathematical_Neurometrix\neuro-3d\application\use_cases\process_volume_use_case.cpp"
with open(path, 'r', encoding='utf-8') as f:
    txt = f.read()
old = "if (sym.peak_value > 30.0) {"
new = "if (sym.peak_value > 15.0) {"
assert old in txt, "threshold no encontrado"
txt = txt.replace(old, new)
with open(path, 'w', encoding='utf-8', newline='\n') as f:
    f.write(txt)
print("THRESHOLD OK")
