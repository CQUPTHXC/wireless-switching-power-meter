# 将include/web/src下的文件转换为C/C++数组,方便编译到flash
Import("env")

import os

# 定义要嵌入的文件
files_to_embed = []
src_dir_path="include/web/src"
header_dir_path="include/web/c_header"

# 清空c_header
def clear_folder(folder_path):
    for root, dirs, files in os.walk(folder_path):
        for file in files:
            file_path = os.path.join(root, file)
            os.remove(file_path)

clear_folder(header_dir_path)
     

# 遍历include/web/src
for root, dirs, files in os.walk(src_dir_path):
    for file in files:
        file_path = os.path.join(root, file)
        #替换空格,-,点为_
        file_name = file.replace(" ","_").replace(".","_").replace("-","_")
        files_to_embed.append((file_name, file_path))
# 将文件转换为 C/C++ 数组
for var_name, file_path in files_to_embed:
    with open(file_path, "r",encoding="utf-8") as f:
        content = f.read()

    # 生成头文件
    header_content="//该文件由embed_files.py自动生成,请勿手动修改\n"
    header_content = header_content+"#ifndef {var}_H\n".format(var=var_name)
    header_content = header_content+"#define {var}_H\n".format(var=var_name)

    header_content = header_content+f"const char {var_name}[] PROGMEM = R\"=====(\n{content}\n)=====\";\n"

    header_content = header_content+"#endif\n"
    
    file_name="/{var}.h".format(var=var_name)
    file_name=header_dir_path+file_name
    # 写入头文件
    with open(file_name, "w", encoding="utf-8") as f:
        f.write(header_content)
    print(f"successfully generated {file_name}")