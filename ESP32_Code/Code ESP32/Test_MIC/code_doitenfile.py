import os
import re

# Đường dẫn đến thư mục chứa file
folder_path = r"C:\Users\ACER\Documents\A_PlatformIO\Test_MIC\TrainningInput_Data"

# Số file chính trong mỗi nhóm
main_files_per_group = 40
# Số file phụ trong mỗi nhóm
extra_files_per_group = 20
# Tổng số file trong mỗi nhóm (file chính + file phụ)
total_files_per_group = main_files_per_group + extra_files_per_group

# Lấy danh sách file và sắp xếp (ưu tiên file chính trước, file phụ sau)
files = sorted(os.listdir(folder_path), key=lambda x: (int(re.findall(r'\d+', x.split('_')[0])[0]), x))

# Bắt đầu đổi tên
current_label = 0
file_index = 1

for file in files:
    if file.startswith("output") and file.endswith(".wav"):
        # Xác định file thuộc nhóm nào
        file_number = int(re.findall(r'\d+', file.split('_')[0])[0])
        if file_number > (current_label + 1) * main_files_per_group:
            current_label += 1
            file_index = 1  # Reset chỉ số file trong nhóm mới

        # Tạo tên mới
        new_name = f"label{current_label}_output{file_index}.wav"
        old_path = os.path.join(folder_path, file)
        new_path = os.path.join(folder_path, new_name)

        # Đổi tên file
        os.rename(old_path, new_path)
        file_index += 1

print("Đổi tên file thành công!")
