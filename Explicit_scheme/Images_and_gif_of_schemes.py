import os
import numpy as np
import matplotlib.pyplot as plt
import imageio.v2 as imageio

# Путь к файлу с параметрами
params_file_path = "params.txt"  # Укажите правильный путь

# Чтение параметров из файла
with open(params_file_path, 'r') as params_file:
    dt = params_file.readline().strip().replace('.', '-')
    num_dx = params_file.readline().strip()
    t_max = int(params_file.readline().strip())

base_path = f"Results/time_step_{dt}_num_elements_{num_dx}_{num_dx}/"
output_img_path = os.path.join(base_path, "imgs_temp_distribution")
color_map = 'viridis'  # Задайте нужную цветовую схему
image_dpi = 300  # Задайте нужное качество изображения

# Создание папки для изображений, если её нет
os.makedirs(output_img_path, exist_ok=True)

def generate_image(data, time, output_path):
    plt.figure(figsize=(6, 5))
    plt.imshow(data, cmap=color_map, interpolation='nearest')
    plt.colorbar(label='Temperature')
    plt.title(f"Temperatures {dt} - Time {time}")
    plt.xlabel("X-axis")
    plt.ylabel("Y-axis")
    plt.savefig(output_path, dpi=image_dpi)  # Сохранение с указанным качеством
    plt.close()

def create_gif(image_paths, output_gif_path):
    frames = []
    for image_path in image_paths:
        frames.append(imageio.imread(image_path))
    imageio.mimsave(output_gif_path, frames, format='GIF', loop=0, duration=0.5)

def main():
    image_paths = []
    for T in range(0, t_max+1, 1):
        file_name = f"time_step_{dt}_Time_{T}_num_elements_{num_dx}.txt"
        file_path = os.path.join(base_path, file_name)
        
        if os.path.isfile(file_path):
            data = np.loadtxt(file_path, skiprows=1)  # Пропускаем первую строку
            output_image_file = os.path.join(output_img_path, f"time_{T}.png")
            generate_image(data, T, output_image_file)
            image_paths.append(output_image_file)
        else:
            print(f"File not found: {file_path}")
    
    if image_paths:
        output_gif_file = os.path.join(output_img_path, "temperature_distribution.gif")
        create_gif(image_paths, output_gif_file)
        print(f"GIF saved to {output_gif_file}")
    else:
        print("No images were created, GIF will not be generated.")

if __name__ == "__main__":
    main()
