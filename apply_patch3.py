path_hpp = r"C:\Users\candu\source\repos\Mathematical_Neurometrix\neuro-3d\domain\services\bilateral_symmetry_3d.hpp"
with open(path_hpp, 'r', encoding='utf-8') as f:
    txt = f.read()

old = """                    const std::uint32_t i1 = z * plane + y * w + x;
                    const std::uint32_t i2 = z * plane + y * w + mx;
                    const double diff = std::abs(
                        static_cast<double>(vol(i1)) -
                        static_cast<double>(vol(i2)));
                    total += diff;
                    if (diff > local_peak) {"""

new = """                    const std::uint32_t i1 = z * plane + y * w + x;
                    const std::uint32_t i2 = z * plane + y * w + mx;
                    const double v1 = static_cast<double>(vol(i1));
                    const double v2 = static_cast<double>(vol(i2));
                    if (v1 < -200.0 || v1 > 400.0) continue;
                    if (v2 < -200.0 || v2 > 400.0) continue;
                    const double diff = std::abs(v1 - v2);
                    total += diff;
                    if (diff > local_peak) {"""

assert old in txt, "bloque simetria no encontrado"
txt = txt.replace(old, new)

with open(path_hpp, 'w', encoding='utf-8', newline='\n') as f:
    f.write(txt)

print("PATCH symmetry OK")
