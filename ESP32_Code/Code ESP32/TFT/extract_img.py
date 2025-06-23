from PIL import Image
import numpy as np

# Đọc ảnh
image_path = "/home/men/Documents/PlatformIO/Projects/TFT/image.png"  # Đường dẫn ảnh
image = Image.open(image_path)

# Giữ nguyên tỷ lệ ảnh, thay đổi kích thước sao cho vừa với kích thước màn hình 128x160
desired_width, desired_height = 50, 50
image.thumbnail((desired_width, desired_height))  # Giữ tỷ lệ gốc

# Đổi kích thước ảnh nếu cần thiết (nếu ảnh nhỏ hơn thì không thay đổi)
image = image.resize((desired_width, desired_height))

# Chuyển ảnh thành RGB
image = image.convert("RGB")

# Lấy các pixel của ảnh
pixels = np.array(image)

# Chuyển đổi các giá trị màu từ (R, G, B) sang RGB565 (16-bit màu)
def rgb_to_rgb565(r, g, b):
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >> 3)

# Chuyển tất cả các pixel thành giá trị RGB565
rgb565_pixels = np.vectorize(rgb_to_rgb565)(pixels[:, :, 0], pixels[:, :, 1], pixels[:, :, 2])

# Chuyển đổi thành dãy các điểm ảnh
flat_pixels = rgb565_pixels.flatten()

# Xuất dãy các điểm ảnh ra file .h
header_file = "image_data.h"

with open(header_file, "w") as f:
    f.write("// Dãy pixel ảnh cho TFT\n")
    f.write("const uint16_t image_data[] = {\n")
    for i, pixel in enumerate(flat_pixels):
        # Nếu là phần tử cuối, không thêm dấu phẩy
        if i == len(flat_pixels) - 1:
            f.write(f"0x{pixel:04X}\n")
        else:
            f.write(f"0x{pixel:04X}, ")
    f.write("};\n")

print(f"Dãy pixel đã được lưu trong file {header_file}")
