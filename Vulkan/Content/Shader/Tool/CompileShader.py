import os
import subprocess

import shutil

def delete_all_files_in_folder(folder_path):
    """
    删除指定文件夹下的所有文件和子文件夹。

    参数:
        folder_path (str): 要清理的文件夹路径。
    """
    # 删除文件夹内容，但保留文件夹本身
    for root, dirs, files in os.walk(folder_path, topdown=False):
        for file in files:
            file_path = os.path.join(root, file)
            os.remove(file_path)
        for dir in dirs:
            dir_path = os.path.join(root, dir)
            os.rmdir(dir_path)
            
def get_file_full_path_names_in_directory(suffix, directory='.', recursive=False):
    file_name_list = []
    if recursive:
        # 递归读取子目录
        for root, _, files in os.walk(directory):
            for file in files:
                file_path = os.path.join(directory, file)
                if file.endswith(suffix):
                    file_name_list.append(os.path.abspath(file_path))
    else:
        # 仅读取指定目录
        for file in os.listdir(directory):
            file_path = os.path.join(directory, file)
            if os.path.isfile(file_path) and file.endswith(suffix):
                file_name_list.append(os.path.abspath(file_path))
                    
    return file_name_list

def get_filename_without_extension(file_path):
    filename = os.path.basename(file_path)
    return os.path.splitext(filename)[0]

def generate_spv_file(file_path_name, spv_file_out_dir):
    file_name = get_filename_without_extension(file_path_name)
    out_file_name = spv_file_out_dir+"/"+file_name+".spv"
    #print("generate_spv_file:"+out_file_name)
    subprocess.run(['D:\\SDK\\VulkanSDK\\Bin\\glslc.exe', file_path_name, '-o', out_file_name]), 
    
def get_current_file_abs_path():
    # 获取当前脚本的绝对路径
    current_file_path = os.path.abspath(__file__)

    # 获取当前脚本所在文件夹的绝对路径
    current_folder_path = os.path.dirname(current_file_path)
    return current_folder_path

def main():
    spv_file_out_dir = get_current_file_abs_path()+"/../Spv/"
    spv_file_out_abs_dir = os.path.abspath(spv_file_out_dir)
    #print("spv_file_out_abs_dir:"+spv_file_out_abs_dir)
    delete_all_files_in_folder(spv_file_out_abs_dir)
    shader_file_names = get_file_full_path_names_in_directory("vert", "./Source")
    shader_file_names.extend(get_file_full_path_names_in_directory("frag", "./Source"))
    for shader_file_name in shader_file_names:
        generate_spv_file(shader_file_name, spv_file_out_abs_dir)
    
if __name__ == "__main__":
    main()

