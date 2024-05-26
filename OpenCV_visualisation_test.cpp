#include <iostream>
#include <fstream>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

int main() {
  // Путь к файлу
  string filename = "Implicit_scheme/Results/time_step_0-1_num_elements_50_50/time_step_0-1_Time_0_num_elements_50.txt";

  // Чтение данных из файла
  ifstream file(filename);
  vector<vector<float>> temperatureData;

  // Пропуск первых строк
  string line;
  getline(file, line);
  getline(file, line);

  while (getline(file, line)) {
    vector<float> row;
    string temp;
    stringstream ss(line);

    // Разделение строки на числа
    while (getline(ss, temp, '\t')) {
      row.push_back(stof(temp));
    }

    temperatureData.push_back(row);
  }
  file.close();

  // Создание изображения
  int width = temperatureData[0].size();
  int height = temperatureData.size();
  Mat image(height, width, CV_8UC3, Scalar(0, 0, 0)); // Черный фон

  // Масштабирование и цветовое кодирование температуры
  float minTemp = 91, maxTemp = 99; // Определение минимальной и максимальной температур

  for (int i = 0; i < height; i++) {
    for (int j = 0; j < width; j++) {
      float normalizedTemp = (temperatureData[i][j] - minTemp) / (maxTemp - minTemp);
      // Используем цветную карту для отображения температуры
      // Пример:
      int blue = static_cast<int>(255 * (1 - normalizedTemp));
      int red = static_cast<int>(255 * normalizedTemp);
      image.at<Vec3b>(i, j) = Vec3b(blue, 0, red);
    }
  }

  // Сохранение изображения
  imwrite("temperature_distribution.png", image);

  return 0;
}